#pragma warning(disable : 4244)
#include "misc_manager.h"

#include <filesystem>
#include <fstream>

#include "codec.h"
#include "database_executor.h"
#include "log.hpp"
#include "message_handler.h"
#include "offsets.h"
#include "rpc_helper.h"
#include "spy.h"
#include "spy_types.h"
#include "util.h"

namespace misc
{
using namespace std;
namespace fs     = std::filesystem;
namespace OsMisc = Offsets::Misc;
namespace OsSns  = Offsets::Misc::Sns;

using get_sns_data_mgr_t          = QWORD (*)();
using get_sns_timeline_mgr_t      = QWORD (*)();
using get_sns_first_page_t        = QWORD (*)(QWORD, QWORD, QWORD);
using get_sns_next_page_scene_t   = QWORD (*)(QWORD, QWORD);
using get_chat_mgr_t              = QWORD (*)();
using new_chat_msg_t              = QWORD (*)(char *);
using free_chat_msg_t             = QWORD (*)(QWORD);
using get_pre_download_mgr_t      = QWORD (*)();
using get_mgr_by_prefix_localid_t = QWORD (*)(QWORD, QWORD);
using push_attach_task_t          = QWORD (*)(QWORD, QWORD, QWORD, QWORD);
using get_ocr_manager_t           = QWORD (*)();
using do_ocr_task_t               = QWORD (*)(QWORD, QWORD, QWORD, QWORD, QWORD, QWORD);
using get_qr_code_mgr_t           = QWORD (*)();

struct ImagePattern {
    uint8_t header1_candidate;
    uint8_t header2_expected;
    const char *extension;
};

static constexpr ImagePattern patterns[] = {
    { 0x89, 0x50, ".png" },
    { 0xFF, 0xD8, ".jpg" },
    { 0x47, 0x49, ".gif" },
};

static std::string detect_image_extension(uint8_t header1, uint8_t header2, uint8_t *key)
{

    for (const auto &pat : patterns) {
        *key = pat.header1_candidate ^ header1;
        if ((pat.header2_expected ^ *key) == header2) {
            return pat.extension;
        }
    }
    LOG_ERROR("未知类型：{:02x} {:02x}", header1, header2);
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
    auto ext    = detect_image_extension(buffer[0], buffer[1], &key);
    if (ext.empty()) {
        LOG_ERROR("无法检测文件类型.");
        return "";
    }

    std::for_each(buffer.begin(), buffer.end(), [key](char &c) { c ^= key; });

    fs::path dst_path = dst_dir / (src.stem().string() + ext);
    if (!fs::exists(dst_dir)) fs::create_directories(dst_dir);

    std::ofstream out(dst_path, std::ios::binary);
    if (!out) {
        LOG_ERROR("写入文件失败: {}", dst_path.generic_string());
        return "";
    }

    out.write(buffer.data(), buffer.size());
    return dst_path.generic_string();
}

static int get_first_page()
{
    int status = -1;

    auto GetSNSDataMgr   = Spy::getFunction<get_sns_data_mgr_t>(OsSns::DATA_MGR);
    auto GetSNSFirstPage = Spy::getFunction<get_sns_first_page_t>(OsSns::FIRST);

    QWORD buff[16] = { 0 };
    QWORD mgr      = GetSNSDataMgr();
    status         = (int)GetSNSFirstPage(mgr, (QWORD)&buff, 1);

    return status;
}

static int get_next_page(QWORD id)
{
    int status = -1;

    auto GetSnsTimeLineMgr   = Spy::getFunction<get_sns_timeline_mgr_t>(OsSns::TIMELINE);
    auto GetSNSNextPageScene = Spy::getFunction<get_sns_next_page_scene_t>(OsSns::NEXT);

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
        LOG_WARN("文件已存在：{}", extra.generic_string());
        return 0;
    }

    if (db::get_local_id_and_dbidx(id, &localId, &dbIdx) != 0) {
        LOG_ERROR("获取 localId 失败, 请检查消息 id: {} 是否正确", to_string(id));
        return status;
    }

    auto NewChatMsg            = Spy::getFunction<new_chat_msg_t>(OsMisc::INSATNCE);
    auto FreeChatMsg           = Spy::getFunction<free_chat_msg_t>(OsMisc::FREE);
    auto GetChatMgr            = Spy::getFunction<get_chat_mgr_t>(OsMisc::CHAT_MGR);
    auto GetPreDownLoadMgr     = Spy::getFunction<get_pre_download_mgr_t>(OsMisc::PRE_DOWNLOAD_MGR);
    auto PushAttachTask        = Spy::getFunction<push_attach_task_t>(OsMisc::PUSH_ATTACH_TASK);
    auto GetMgrByPrefixLocalId = Spy::getFunction<get_mgr_by_prefix_localid_t>(OsMisc::PRE_LOCAL_ID_MGR);

    LARGE_INTEGER l;
    l.HighPart = dbIdx;
    l.LowPart  = (DWORD)localId;

    char *buff = util::AllocBuffer<char>(0x460);
    if (buff == nullptr) {
        LOG_ERROR("申请内存失败.");
        return status;
    }

    QWORD pChatMsg = NewChatMsg(buff);
    GetChatMgr();
    GetMgrByPrefixLocalId(l.QuadPart, pChatMsg);
    QWORD type = util::get_dword(reinterpret_cast<QWORD>(buff) + 0x38);

    fs::path save_path, thumb_path;
    switch (type) {
        case 0x03: { // Image: extra
            save_path = extra;
            break;
        }
        case 0x3E:
        case 0x2B: { // Video: thumb
            thumb_path = thumb;
            save_path  = fs::path(thumb).replace_extension("mp4");
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

    LOG_DEBUG("保存路径: {}", save_path.generic_string());
    // 创建父目录，由于路径来源于微信，不做检查
    fs::create_directory(save_path.parent_path());

    int temp           = 1;
    auto wx_save_path  = util::CreateWxString(save_path.make_preferred().string());
    auto wx_thumb_path = util::CreateWxString(thumb_path.make_preferred().string());

    memcpy(&buff[0x280], wx_thumb_path, sizeof(WxString));
    memcpy(&buff[0x2A0], wx_save_path, sizeof(WxString));
    memcpy(&buff[0x40C], &temp, sizeof(temp));

    QWORD mgr = GetPreDownLoadMgr();
    status    = (int)PushAttachTask(mgr, pChatMsg, 0, 1);
    FreeChatMsg(pChatMsg);
    util::FreeBuffer(buff);

    return status;
}

std::string get_audio(uint64_t id, const fs::path &dir)
{
    if (!fs::exists(dir)) fs::create_directories(dir);

    fs::path mp3path = dir / (std::to_string(id) + ".mp3");
    if (fs::exists(mp3path)) return mp3path.generic_string();

    auto silk = db::get_audio_data(id);
    if (silk.empty()) {
        LOG_ERROR("没有获取到语音数据.");
        return "";
    }

    Silk2Mp3(silk, mp3path.generic_string(), 24000);
    return mp3path.generic_string();
}

std::string get_pcm_audio(uint64_t id, const fs::path &dir, int32_t sr)
{
    if (!fs::exists(dir)) fs::create_directories(dir);

    fs::path pcmpath = dir / (std::to_string(id) + ".pcm");
    if (fs::exists(pcmpath)) return pcmpath.generic_string();

    auto silk = db::get_audio_data(id);
    if (silk.empty()) {
        LOG_ERROR("没有获取到语音数据.");
        return "";
    }

    std::vector<uint8_t> pcm;
    SilkDecode(silk, pcm, sr);

    std::ofstream out(pcmpath, std::ios::binary);
    if (!out) {
        LOG_ERROR("创建文件失败: {}", pcmpath.generic_string());
        return "";
    }

    out.write(reinterpret_cast<char *>(pcm.data()), pcm.size());
    return pcmpath.generic_string();
}

OcrResult_t get_ocr_result(const fs::path &path)
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
    std::string uri;
    auto get_qr_code_mgr = Spy::getFunction<get_qr_code_mgr_t>(OsMisc::QR_CODE);

    uint64_t addr = get_qr_code_mgr() + 0x68;
    uint64_t len  = *(uint64_t *)(addr + 0x10);
    if (len == 0) {
        LOG_ERROR("获取二维码失败.");
        return uri;
    }

    if (*(uint64_t *)(addr + 0x18) == 0xF) {
        uri = std::string((char *)addr, len);
    } else {
        uri = std::string(*(char **)(addr), len);
    }

    return "http://weixin.qq.com/x/" + uri;
}

int receive_transfer(const std::string &wxid, const std::string &transferid, const std::string &transactionid)
{
    LOG_ERROR("技术太菜，实现不了。");
    return -1;
}

bool rpc_get_audio(const AudioMsg &am, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_AUDIO_MSG>(
        out, len, [&](Response &rsp) { rsp.msg.str = (char *)get_audio(am.id, am.dir).c_str(); });
}

bool rpc_get_pcm_audio(uint64_t id, const fs::path &dir, int32_t sr, uint8_t *out, size_t *len) { return false; }

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
    int status = -1;
    if (att.thumb || att.extra) {
        std::string thumb = att.thumb ? att.thumb : "";
        std::string extra = att.extra ? att.extra : "";
        status            = download_attachment(att.id, thumb, extra);
    } else {
        LOG_ERROR("文件地址不能全为空");
    }

    return fill_response<Functions_FUNC_DOWNLOAD_ATTACH>(out, len, [&](Response &rsp) { rsp.msg.status = status; });
}

bool rpc_revoke_message(uint64_t id, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_REVOKE_MSG>(out, len,
                                                    [&](Response &rsp) { rsp.msg.status = revoke_message(id); });
}

bool rpc_get_ocr_result(const fs::path &path, uint8_t *out, size_t *len)
{
    auto ret = get_ocr_result(path);
    return fill_response<Functions_FUNC_EXEC_OCR>(out, len, [&](Response &rsp) {
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
