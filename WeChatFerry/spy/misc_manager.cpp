#pragma warning(disable : 4244)
#include "misc_manager.h"

#include <filesystem>
#include <fstream>

#include "framework.h"

#include "codec.h"
#include "database_executor.h"
#include "log.hpp"
#include "message_handler.h"
#include "rpc_helper.h"
#include "spy_types.h"
#include "util.h"

using namespace std;
namespace fs = std::filesystem;

extern QWORD g_WeChatWinDllAddr;

namespace misc
{
#define HEADER_PNG1 0x89
#define HEADER_PNG2 0x50
#define HEADER_JPG1 0xFF
#define HEADER_JPG2 0xD8
#define HEADER_GIF1 0x47
#define HEADER_GIF2 0x49

#define OS_GET_SNS_DATA_MGR           0x21E2200
#define OS_GET_SNS_FIRST_PAGE         0x2E212D0
#define OS_GET_SNS_TIMELINE_MGR       0x2DB3390
#define OS_GET_SNS_NEXT_PAGE          0x2EC8970
#define OS_NEW_CHAT_MSG               0x1B5E140
#define OS_FREE_CHAT_MSG              0x1B55850
#define OS_GET_CHAT_MGR               0x1B876C0
#define OS_GET_MGR_BY_PREFIX_LOCAL_ID 0x213FB00
#define OS_GET_PRE_DOWNLOAD_MGR       0x1C0EE70
#define OS_PUSH_ATTACH_TASK           0x1CDF4E0
#define OS_LOGIN_QR_CODE              0x59620D8

using get_sns_data_mgr_t          = QWORD (*)();
using get_sns_timeline_mgr_t      = QWORD (*)();
using get_sns_first_page_t        = QWORD (*)(QWORD, QWORD, QWORD);
using get_sns_next_page_scene_t   = QWORD (*)(QWORD, QWORD);
using get_chat_mgr_t              = QWORD (*)();
using new_chat_msg_t              = QWORD (*)(QWORD);
using free_chat_msg_t             = QWORD (*)(QWORD);
using get_pre_download_mgr_t      = QWORD (*)();
using get_mgr_by_prefix_localid_t = QWORD (*)(QWORD, QWORD);
using push_attach_task_t          = QWORD (*)(QWORD, QWORD, QWORD, QWORD);
using get_ocr_manager_t           = QWORD (*)();
using do_ocr_task_t               = QWORD (*)(QWORD, QWORD, QWORD, QWORD, QWORD, QWORD);

static std::string detect_image_extension(uint8_t header1, uint8_t header2, uint8_t &key)
{
    if ((key = HEADER_PNG1 ^ header1) && (HEADER_PNG2 ^ key) == header2) return ".png";
    if ((key = HEADER_JPG1 ^ header1) && (HEADER_JPG2 ^ key) == header2) return ".jpg";
    if ((key = HEADER_GIF1 ^ header1) && (HEADER_GIF2 ^ key) == header2) return ".gif";
    return "";
}

std::string decrypt_image(const fs::path &src, const fs::path &dst_dir)
{
    if (!fs::exists(src)) {
        LOG_ERROR("文件不存在: {}", src.string());
        return "";
    }

    std::ifstream in(src, std::ios::binary);
    if (!in) {
        LOG_ERROR("无法打开文件: {}", src.string());
        return "";
    }

    std::vector<char> buffer(std::istreambuf_iterator<char>(in), {});
    if (buffer.size() < 2) return "";

    uint8_t key = 0x00;
    auto ext    = detect_image_extension(buffer[0], buffer[1], key);
    if (!ext.empty()) {
        LOG_ERROR("无法检测文件类型.");
        return "";
    }

    std::for_each(buffer.begin(), buffer.end(), [key](char &c) { c ^= key; });

    fs::path dst_path = dst_dir / (src.stem().string() + ext);
    if (!fs::exists(dst_dir)) fs::create_directories(dst_dir);

    std::ofstream out(dst_path, std::ios::binary);
    if (!out) {
        LOG_ERROR("写入文件失败: {}", dst_path.string());
        return "";
    }

    out.write(buffer.data(), buffer.size());
    return dst_path.string();
}

static int get_first_page()
{
    int status = -1;

    get_sns_data_mgr_t GetSNSDataMgr     = (get_sns_data_mgr_t)(g_WeChatWinDllAddr + OS_GET_SNS_DATA_MGR);
    get_sns_first_page_t GetSNSFirstPage = (get_sns_first_page_t)(g_WeChatWinDllAddr + OS_GET_SNS_FIRST_PAGE);

    QWORD buff[16] = { 0 };
    QWORD mgr      = GetSNSDataMgr();
    status         = (int)GetSNSFirstPage(mgr, (QWORD)buff, 1);

    return status;
}

static int get_next_page(QWORD id)
{
    int status = -1;

    get_sns_timeline_mgr_t GetSnsTimeLineMgr = (get_sns_timeline_mgr_t)(g_WeChatWinDllAddr + OS_GET_SNS_TIMELINE_MGR);
    get_sns_next_page_scene_t GetSNSNextPageScene
        = (get_sns_next_page_scene_t)(g_WeChatWinDllAddr + OS_GET_SNS_NEXT_PAGE);

    QWORD mgr = GetSnsTimeLineMgr();
    status    = (int)GetSNSNextPageScene(mgr, id);

    return status;
}

int refresh_pyq(uint64_t id)
{
    auto &msgHandler = message::Handler::getInstance();
    if (!msgHandler.isPyqListening()) {
        LOG_ERROR("没有启动朋友圈消息接收，参考：enable_receiving_msg");
        return -1;
    }

    if (id == 0) {
        return get_first_page();
    }

    return get_next_page(id);
}

/*******************************************************************************
 * 都说我不写注释，写一下吧
 * 其实也没啥好写的，就是下载资源
 * 主要介绍一下几个参数：
 * id：好理解，消息 id
 * thumb：图片或者视频的缩略图路径；如果是视频，后缀为 mp4 后就是存在路径了
 * extra：图片、文件的路径
 *******************************************************************************/
int download_attachment(uint64_t id, const fs::path &thumb, const fs::path &extra)
{
    int status = -1;
    QWORD localId;
    uint32_t dbIdx;

    if (fs::exists(extra)) { // 第一道，不重复下载。TODO: 通过文件大小来判断
        return 0;
    }

    if (db::get_local_id_and_dbidx(id, &localId, &dbIdx) != 0) {
        LOG_ERROR("获取 localId 失败, 请检查消息 id: {} 是否正确", to_string(id));
        return status;
    }

    new_chat_msg_t NewChatMsg                = (new_chat_msg_t)(g_WeChatWinDllAddr + OS_NEW_CHAT_MSG);
    free_chat_msg_t FreeChatMsg              = (free_chat_msg_t)(g_WeChatWinDllAddr + OS_FREE_CHAT_MSG);
    get_chat_mgr_t GetChatMgr                = (get_chat_mgr_t)(g_WeChatWinDllAddr + OS_GET_CHAT_MGR);
    get_pre_download_mgr_t GetPreDownLoadMgr = (get_pre_download_mgr_t)(g_WeChatWinDllAddr + OS_GET_PRE_DOWNLOAD_MGR);
    push_attach_task_t PushAttachTask        = (push_attach_task_t)(g_WeChatWinDllAddr + OS_PUSH_ATTACH_TASK);
    get_mgr_by_prefix_localid_t GetMgrByPrefixLocalId
        = (get_mgr_by_prefix_localid_t)(g_WeChatWinDllAddr + OS_GET_MGR_BY_PREFIX_LOCAL_ID);

    LARGE_INTEGER l;
    l.HighPart = dbIdx;
    l.LowPart  = (DWORD)localId;

    char *buff = (char *)HeapAlloc(GetProcessHeap(), 0, 0x460);
    if (buff == nullptr) {
        LOG_ERROR("申请内存失败.");
        return status;
    }

    QWORD pChatMsg = NewChatMsg((QWORD)buff);
    GetChatMgr();
    GetMgrByPrefixLocalId(l.QuadPart, pChatMsg);

    QWORD type = util::get_qword(reinterpret_cast<QWORD>(buff) + 0x38);

    fs::path save_path, thumb_path;
    switch (type) {
        case 0x03: { // Image: extra
            save_path = extra;
            break;
        }
        case 0x3E:
        case 0x2B: { // Video: thumb
            thumb_path = thumb;
            save_path  = fs::path(thumb).replace_extension("mp4").string();
            break;
        }
        case 0x31: { // File: extra
            save_path = extra;
            break;
        }
        default:
            LOG_ERROR("不支持的文件类型: {}", type);
            return -2;
    }

    if (fs::exists(save_path)) { // 不重复下载。TODO: 通过文件大小来判断
        return 0;
    }

    LOG_DEBUG("保存路径: {}", save_path.string());
    // 创建父目录，由于路径来源于微信，不做检查
    fs::create_directory(save_path.parent_path());

    int temp           = 1;
    auto wx_save_path  = util::new_wx_string(save_path.string());
    auto wx_thumb_path = util::new_wx_string(thumb_path.string());

    memcpy(&buff[0x280], wx_thumb_path.get(), sizeof(WxString));
    memcpy(&buff[0x2A0], wx_save_path.get(), sizeof(WxString));
    memcpy(&buff[0x40C], &temp, sizeof(temp));

    QWORD mgr = GetPreDownLoadMgr();
    status    = (int)PushAttachTask(mgr, pChatMsg, 0, 1);
    FreeChatMsg(pChatMsg);

    return status;
}

std::string get_audio(uint64_t id, const fs::path &dir)
{
    if (!fs::exists(dir)) fs::create_directories(dir);

    fs::path mp3path = dir / (std::to_string(id) + ".mp3");
    if (fs::exists(mp3path)) return mp3path.string();

    auto silk = db::get_audio_data(id);
    if (silk.empty()) {
        LOG_ERROR("没有获取到语音数据.");
        return "";
    }

    Silk2Mp3(silk, mp3path.string(), 24000);
    return mp3path.string();
}

std::string get_pcm_audio(uint64_t id, const fs::path &dir, int32_t sr)
{
    if (!fs::exists(dir)) fs::create_directories(dir);

    fs::path pcmpath = dir / (std::to_string(id) + ".pcm");
    if (fs::exists(pcmpath)) return pcmpath.string();

    auto silk = db::get_audio_data(id);
    if (silk.empty()) {
        LOG_ERROR("没有获取到语音数据.");
        return "";
    }

    std::vector<uint8_t> pcm;
    SilkDecode(silk, pcm, sr);

    std::ofstream out(pcmpath, std::ios::binary);
    if (!out) {
        LOG_ERROR("创建文件失败: {}", pcmpath.string());
        return "";
    }

    out.write(reinterpret_cast<char *>(pcm.data()), pcm.size());
    return pcmpath.string();
}

OcrResult_t get_ocr_result(const std::filesystem::path &path)
{
    OcrResult_t ret = { -1, "" };
#if 0 // 参数没调好，会抛异常，看看有没有好心人来修复
    if (!fs::exists(path)) {
        LOG_ERROR("Can not find: {}", path);
        return ret;
    }

    get_ocr_manager_t GetOCRManager = (get_ocr_manager_t)(g_WeChatWinDllAddr + 0x1D6C3C0);
    do_ocr_task_t DoOCRTask         = (do_ocr_task_t)(g_WeChatWinDllAddr + 0x2D10BC0);

    QWORD unk1 = 0, unk2 = 0, unused = 0;
    QWORD *pUnk1 = &unk1;
    QWORD *pUnk2 = &unk2;
    // 路径分隔符有要求，必须为 `\`
    wstring wsPath = util::s2w(fs::path(path).make_preferred().string());
    WxString wxPath(wsPath);
    vector<QWORD> *pv = (vector<QWORD> *)HeapAlloc(GetProcessHeap(), 0, 0x20);
    RawVector_t *pRv  = (RawVector_t *)pv;
    pRv->finish       = pRv->start;
    char buff[0x98]   = { 0 };
    memcpy(buff, &pRv->start, sizeof(QWORD));

    QWORD mgr  = GetOCRManager();
    ret.status = (int)DoOCRTask(mgr, (QWORD)&wxPath, unused, (QWORD)buff, (QWORD)&pUnk1, (QWORD)&pUnk2);

    QWORD count = util::get_qword(buff + 0x8);
    if (count > 0) {
        QWORD header = util::get_qword(buff);
        for (QWORD i = 0; i < count; i++) {
            QWORD content = util::get_qword(header);
            ret.result += util::w2s(get_pp_wstring(content + 0x28));
            ret.result += "\n";
            header = content;
        }
    }
#endif
    return ret;
}

int revoke_message(uint64_t id)
{
    int status = -1;
#if 0 // 这个挺鸡肋的，因为自己发的消息没法直接获得 msgid，就这样吧
    QWORD localId;
    uint32_t dbIdx;
    if (GetLocalIdandDbidx(id, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", to_string(id));
        return status;
    }
#endif
    return status;
}

std::string get_login_url()
{
    LPVOID targetAddress = reinterpret_cast<LPBYTE>(g_WeChatWinDllAddr) + OS_LOGIN_QR_CODE;
    char *dataPtr        = *reinterpret_cast<char **>(targetAddress);
    if (!dataPtr) {
        LOG_ERROR("获取二维码失败.");
        return "";
    }
    return "http://weixin.qq.com/x/" + std::string(dataPtr, 22);
}

int receive_transfer(const std::string &wxid, const std::string &transferid, const std::string &transactionid)
{
    // 别想了，这个不实现了
    return -1;
}

bool rpc_get_audio(const AudioMsg &am, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_AUDIO_MSG>(
        out, len, [&](Response &rsp) { rsp.msg.str = (char *)get_audio(am.id, am.dir).c_str(); });
}

bool rpc_get_pcm_audio(uint64_t id, const std::filesystem::path &dir, int32_t sr, uint8_t *out, size_t *len)
{
    return false;
}

bool rpc_decrypt_image(const DecPath &dec, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_DECRYPT_IMAGE>(
        out, len, [&](Response &rsp) { rsp.msg.str = (char *)decrypt_image(dec.src, dec.dst).c_str(); });
}

bool rpc_get_login_url(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_REFRESH_QRCODE>(
        out, len, [&](Response &rsp) { rsp.msg.str = (char *)get_login_url().c_str(); });
}

bool rpc_refresh_pyq(uint64_t id, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_REFRESH_PYQ>(out, len,
                                                     [&](Response &rsp) { rsp.msg.status = refresh_pyq(id); });
}

bool rpc_download_attachment(const AttachMsg &att, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_DOWNLOAD_ATTACH>(
        out, len, [&](Response &rsp) { rsp.msg.status = download_attachment(att.id, att.thumb, att.extra); });
}

bool rpc_revoke_message(uint64_t id, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_REVOKE_MSG>(out, len,
                                                    [&](Response &rsp) { rsp.msg.status = revoke_message(id); });
}

bool rpc_get_ocr_result(const std::filesystem::path &path, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_EXEC_OCR>(out, len, [&](Response &rsp) {
        OcrResult_t ret    = { -1, "" };
        ret                = get_ocr_result(path);
        rsp.msg.ocr.status = ret.status;
        rsp.msg.ocr.result = (char *)ret.result.c_str();
    });
}

bool rpc_receive_transfer(const Transfer &tf, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_RECV_TRANSFER>(
        out, len, [&](Response &rsp) { rsp.msg.status = receive_transfer(tf.wxid, tf.tfid, tf.taid); });
}
} // namespace misc
