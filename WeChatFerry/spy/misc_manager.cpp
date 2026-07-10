#pragma warning(disable : 4244)

#include "misc_manager.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

#include "codec.h"
#include "database_executor.h"
#include "framework.h"
#include "log.hpp"
#include "offsets.h"
#include "pb_util.h"
#include "rpc_helper.h"
#include "spy_types.h"
#include "util.h"

#define HEADER_PNG1 0x89
#define HEADER_PNG2 0x50
#define HEADER_JPG1 0xFF
#define HEADER_JPG2 0xD8
#define HEADER_GIF1 0x47
#define HEADER_GIF2 0x49

namespace fs = std::filesystem;

extern bool gIsListeningPyq;
extern uint32_t g_WeChatWinDllAddr;

namespace
{

using ManagerGetterFn        = void *(*)();
using BufferInitFn           = void(__thiscall *)(void *buffer);
using BufferCleanupFn        = void(__thiscall *)(void *buffer);
using WarmupFn               = void (*)();
using RefreshFirstPageFn     = int(__thiscall *)(void *manager, int forward);
using RefreshNextPageFn      = int(__thiscall *)(void *manager, uint32_t id_low, uint32_t id_high);
using RunOcrFn               = int(__thiscall *)(void *manager, const WxString *path, int reserved,
                                                 void *result_list, uint32_t *found_flag,
                                                 const WxString *null_obj);
using OperatorNewFn          = void *(__cdecl *)(size_t);
using OcrResultDtorFn        = void(__thiscall *)(void *result_list);
using RefreshLoginQrCodeFn   = void(__thiscall *)(void *manager);
using LoadAttachmentMetaFn   = int(__thiscall *)(void *buffer, uint32_t local_id, uint32_t db_idx);
using DownloadAttachmentFn   = int(__thiscall *)(void *manager, void *buffer, int reserved, int sync,
                                                 int user_clicked);
using WxStringAssignFn       = void *(__thiscall *)(void *dest, const wchar_t *src, int len);
using RevokeMessageFn        = int(__thiscall *)(void *manager, void *chat_msg);
using ReceiveTransferFn      = int(__fastcall *)(void *pay_info, const WxString *wxid, uint64_t scratch,
                                                 int confirm);

std::string get_key(uint8_t header1, uint8_t header2, uint8_t *key)
{
    *key = HEADER_PNG1 ^ header1;
    if ((HEADER_PNG2 ^ *key) == header2) {
        return ".png";
    }

    *key = HEADER_JPG1 ^ header1;
    if ((HEADER_JPG2 ^ *key) == header2) {
        return ".jpg";
    }

    *key = HEADER_GIF1 ^ header1;
    if ((HEADER_GIF2 ^ *key) == header2) {
        return ".gif";
    }

    return "";
}

int get_first_page()
{
    int rv = -1;

    auto getMomentsManager = reinterpret_cast<ManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Moments::MGR_GETTER);
    auto requestFirstPage  = reinterpret_cast<RefreshFirstPageFn>(g_WeChatWinDllAddr + Offsets::Moments::GET_FIRST_PAGE);

    void *manager = getMomentsManager();
    if (manager != nullptr) {
        rv = requestFirstPage(manager, 1);
    }

    return rv;
}

int get_next_page(uint64_t id)
{
    int rv = -1;

    auto getMomentsManager = reinterpret_cast<ManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Moments::MGR_GETTER);
    auto requestNextPage   = reinterpret_cast<RefreshNextPageFn>(g_WeChatWinDllAddr + Offsets::Moments::GET_NEXT_PAGE);

    void *manager = getMomentsManager();
    if (manager != nullptr) {
        rv = requestNextPage(manager, static_cast<uint32_t>(id), static_cast<uint32_t>(id >> 32));
    }

    return rv;
}

} // namespace

namespace misc
{

std::string decrypt_image(const std::string &src, const std::string &dir)
{
    // 微信图片是原图逐字节异或同一单字节密钥；由前两字节与已知图片头（PNG/JPG/GIF）反推密钥
    if (!fs::exists(src)) {
        return "";
    }

    std::ifstream in(src.c_str(), std::ios::binary);
    if (!in.is_open()) {
        LOG_ERROR("Failed to read file {}", src);
        return "";
    }

    std::filebuf *pfb = in.rdbuf();
    size_t size       = pfb->pubseekoff(0, std::ios::end, std::ios::in);
    pfb->pubseekpos(0, std::ios::in);

    std::vector<char> buff;
    buff.resize(size);
    char *pBuf = buff.data();
    pfb->sgetn(pBuf, size);
    in.close();

    if (size < 2) {
        LOG_ERROR("File too small: {}", src);
        return "";
    }

    uint8_t key     = 0x00;
    std::string ext = get_key(pBuf[0], pBuf[1], &key);
    if (ext.empty()) {
        LOG_ERROR("Failed to get key.");
        return "";
    }

    for (size_t i = 0; i < size; i++) {
        pBuf[i] ^= key;
    }

    std::string dst = "";

    try {
        if (dir.empty()) {
            dst = fs::path(src).replace_extension(ext).string();
        } else {
            dst = (dir.back() == '\\' || dir.back() == '/') ? dir : (dir + "/");
            dst += fs::path(src).stem().string() + ext;
        }

        std::replace(dst.begin(), dst.end(), '\\', '/');
    } catch (const std::exception &e) {
        LOG_ERROR(util::gb2312_to_utf8(e.what()));
    } catch (...) {
        LOG_ERROR("Unknow exception.");
        return "";
    }

    std::ofstream out(dst.c_str(), std::ios::binary);
    if (!out.is_open()) {
        LOG_ERROR("Failed to write file {}", dst);
        return "";
    }

    out.write(pBuf, size);
    out.close();

    return dst;
}

int refresh_pyq(uint64_t id)
{
    if (!gIsListeningPyq) {
        LOG_ERROR("没有启动朋友圈消息接收，参考：enable_receiving_msg");
        return -1;
    }

    if (id == 0) {
        return get_first_page();
    }

    return get_next_page(id);
}

int download_attachment(uint64_t id, const std::string &thumb, const std::string &extra)
{
    if (g_WeChatWinDllAddr == 0) {
        LOG_ERROR("WeChatWin.dll not located.");
        return -1;
    }

    int status = -1;
    uint64_t localId;
    uint32_t dbIdx;

    if (fs::exists(extra)) {
        return 0;
    }

    if (db::get_local_id_and_dbidx(id, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", to_string(id));
        return status;
    }

    // 加载消息进 ChatMsg → 依类型算落盘路径写入其字段 → 提交下载 → 析构
    char buff[0x2D8] = { 0 };
    auto initChatMsg  = reinterpret_cast<BufferInitFn>(g_WeChatWinDllAddr + Offsets::Message::Send::CHATMSG_CTOR);
    auto warmup       = reinterpret_cast<WarmupFn>(g_WeChatWinDllAddr + Offsets::Attachment::WARMUP);
    auto loadMsg      = reinterpret_cast<LoadAttachmentMetaFn>(g_WeChatWinDllAddr + Offsets::Attachment::LOAD_MSG);
    auto getManager   = reinterpret_cast<ManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Attachment::MGR_GETTER);
    auto pushTask     = reinterpret_cast<DownloadAttachmentFn>(g_WeChatWinDllAddr + Offsets::Attachment::PUSH_TASK);
    auto assignPath   = reinterpret_cast<WxStringAssignFn>(g_WeChatWinDllAddr + Offsets::RichText::ASSIGN);
    auto destroyMsg   = reinterpret_cast<BufferCleanupFn>(g_WeChatWinDllAddr + Offsets::Message::Send::CHATMSG_DTOR);

    initChatMsg(buff);
    warmup();
    loadMsg(buff, static_cast<uint32_t>(localId), dbIdx);

    uint32_t type = util::get_dword((uint32_t)(buff + Offsets::Message::Receive::TYPE));

    std::string save_path  = "";
    std::string thumb_path = "";

    switch (type) {
        case 0x03:  // 图片
            save_path = extra;
            break;
        case 0x3E:  // 视频
        case 0x2B:
            thumb_path = thumb;
            save_path  = fs::path(thumb).replace_extension("mp4").string();
            break;
        case 0x31:  // 文件
            save_path = extra;
            break;
        default:
            break;
    }

    if (fs::exists(save_path)) {
        destroyMsg(buff);
        return 0;
    }

    LOG_DEBUG("path: {}", save_path);
    fs::create_directory(fs::path(save_path).parent_path().string());

    std::wstring wsSavePath  = util::s2w(save_path);
    std::wstring wsThumbPath = util::s2w(thumb_path);

    // 路径字段会被 ~ChatMsg mm_free，须用 ASSIGN 建 WeChat 拥有副本；init 标志置 1 跳过重复子对象初始化
    assignPath(buff + Offsets::Attachment::F_THUMB_PATH, wsThumbPath.c_str(), -1);
    assignPath(buff + Offsets::Attachment::F_SAVE_PATH, wsSavePath.c_str(), -1);
    *reinterpret_cast<int *>(buff + Offsets::Attachment::F_INITED_FLAG) = 1;

    void *manager = getManager();
    if (manager != nullptr) {
        status = pushTask(manager, buff, 0, 1, 0);
    }
    destroyMsg(buff);

    return status;
}

int revoke_message(uint64_t id)
{
    int status = -1;
    uint64_t localId;
    uint32_t dbIdx;
    if (db::get_local_id_and_dbidx(id, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", to_string(id));
        return status;
    }

    // ChatMsg 精确 0x2D8 字节（同发送侧），栈上构造后按 localId/dbIdx 加载再撤回。
    char chat_msg[0x2D8] = { 0 };

    auto initChatMsg     = reinterpret_cast<BufferInitFn>(g_WeChatWinDllAddr + Offsets::Message::Send::CHATMSG_CTOR);
    auto loadMsg         = reinterpret_cast<LoadAttachmentMetaFn>(g_WeChatWinDllAddr + Offsets::Attachment::LOAD_MSG);
    auto getRevokeMgr    = reinterpret_cast<ManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Revoke::MGR_GETTER);
    auto revokeMessageFn = reinterpret_cast<RevokeMessageFn>(g_WeChatWinDllAddr + Offsets::Revoke::REVOKE_MSG);
    auto destroyChatMsg  = reinterpret_cast<BufferCleanupFn>(g_WeChatWinDllAddr + Offsets::Message::Send::CHATMSG_DTOR);

    initChatMsg(chat_msg);
    // GetMgrByPrefixLocalId 内部自初始化 ChatMgr，无需单独 warmup
    loadMsg(chat_msg, static_cast<uint32_t>(localId), dbIdx);

    void *manager = getRevokeMgr();
    if (manager != nullptr) {
        status = revokeMessageFn(manager, chat_msg);
    }
    destroyChatMsg(chat_msg);

    return status;
}

std::string get_audio(uint64_t id, const std::string &dir)
{
    // 语音存于 MediaMSGn.db 的 Media 表（Buf 列为 silk 编码，首字节为标志需跳过）；取出后转 mp3 落盘。
    // 依赖数据库层的 MSG 多库枚举（MultiDBMsgMgr），否则 get_audio_data 查不到 MediaMSG 库。
    if (dir.empty()) {
        LOG_ERROR("Empty dir.");
        return "";
    }

    std::string mp3path = (dir.back() == '\\' || dir.back() == '/') ? dir : (dir + "/");
    mp3path += to_string(id) + ".mp3";
    std::replace(mp3path.begin(), mp3path.end(), '\\', '/');
    if (fs::exists(mp3path)) {
        return mp3path;
    }

    std::vector<uint8_t> silk = db::get_audio_data(id);
    if (silk.empty()) {
        LOG_ERROR("Empty audio data.");
        return "";
    }

    if (Silk2Mp3(silk, mp3path, 24000) != 0 || !fs::exists(mp3path)) {
        return "";
    }

    return mp3path;
}

// OCR 结果链表头：std::list 头（哨兵指针 + 计数）后接缓存命中时被回填的标量字段，须留足空间避免越界
struct OcrResultList {
    void    *head;  // +0x00 哨兵结点（自环空链表）
    uint32_t count; // +0x04 结点数
    uint32_t f8;    // +0x08 缓存命中回填
    uint32_t fc;    // +0x0C
    uint64_t f10;   // +0x10
    uint64_t pad;   // +0x18 冗余，覆盖缓存命中的整段写入
};

OcrResult_t get_ocr_result(const std::string &path)
{
    OcrResult_t ret = { -1, "" };

    if (!fs::exists(path)) {
        LOG_ERROR("Can not find: {}", path);
        return ret;
    }

    std::wstring wsPath = util::s2w(fs::path(path).make_preferred().string());

    WxString wxPath(wsPath);
    WxString nullObj;

    auto opNew         = reinterpret_cast<OperatorNewFn>(g_WeChatWinDllAddr + Offsets::OCR::RESULT_NEW);
    auto getOcrManager = reinterpret_cast<ManagerGetterFn>(g_WeChatWinDllAddr + Offsets::OCR::MGR_GETTER);
    auto runOcr        = reinterpret_cast<RunOcrFn>(g_WeChatWinDllAddr + Offsets::OCR::RUN_OCR);
    auto destroyList   = reinterpret_cast<OcrResultDtorFn>(g_WeChatWinDllAddr + Offsets::OCR::RESULT_DTOR);

    // 构造合法空链表：哨兵结点自环（与 WeChat 内联初始化一致），
    // 缓存命中路径 DoOCRTask 会 splice 结点，无有效哨兵会崩。
    OcrResultList buffer = { 0 };
    void **sentinel      = static_cast<void **>(opNew(0x58));
    if (sentinel == nullptr) {
        LOG_ERROR("Failed to alloc OCR result buffer.");
        return ret;
    }
    sentinel[0]  = sentinel; // next -> self
    sentinel[1]  = sentinel; // prev -> self
    buffer.head  = sentinel;

    uint32_t found = 0;
    int status     = -1;

    void *manager = getOcrManager();
    if (manager != nullptr) {
        status = runOcr(manager, &wxPath, 0, &buffer, &found, &nullObj);
    }

    // DoOCRTask 缓存命中返回 0 并同步回填链表；异步入队返回非 0 task_id（此时无同步结果）。
    if (status != 0) {
        destroyList(&buffer);
        return ret;
    }

    ret.status = status;

    uint32_t addr   = reinterpret_cast<uint32_t>(&buffer);
    uint32_t header = util::get_dword(addr);       // 哨兵/首结点
    uint32_t num    = util::get_dword(addr + 0x4); // 结点数
    for (uint32_t i = 0; i < num; i++) {
        uint32_t content = util::get_dword(header);                     // 下一结点
        ret.result += util::w2s(util::get_wstring(content + 0x14));     // 结点内文本 WxString.wptr
        ret.result += "\n";
        header = content;
    }

    destroyList(&buffer);
    return ret;
}

std::string get_login_url()
{
    if (g_WeChatWinDllAddr == 0) {
        LOG_ERROR("WeChatWin.dll not located.");
        return "";
    }

    // 已登录则无二维码可刷（SERVICE != 0 即在线，与 account::is_logged_in 语义一致）。
    if (util::get_dword(g_WeChatWinDllAddr + Offsets::Account::SERVICE) != 0) {
        LOG_DEBUG("Already logged in, no QR code to refresh.");
        return "";
    }

    // getQRCodeImage 经 doScene 异步获取，完成后把登录 uuid 回填到管理器 +8 的 std::string
    auto getQrCodeManager = reinterpret_cast<ManagerGetterFn>(g_WeChatWinDllAddr + Offsets::QRCode::MGR_GETTER);
    auto getQrCodeImage   = reinterpret_cast<RefreshLoginQrCodeFn>(g_WeChatWinDllAddr + Offsets::QRCode::GET_QRCODE);

    void *manager = getQrCodeManager();
    if (manager == nullptr) {
        LOG_ERROR("Failed to get QRCodeLoginMgr.");
        return "";
    }
    getQrCodeImage(manager);

    // uuid 存于管理器 +8 的 MSVC std::string（size@+0x10、cap@+0x14）；异步回填故轮询 size。
    uint32_t strAddr = g_WeChatWinDllAddr + Offsets::QRCode::URL;
    uint8_t cnt      = 0;
    while (util::get_dword(strAddr + 0x10) == 0) {
        if (cnt > 5) {
            LOG_ERROR("Refresh QR Code timeout.");
            return "";
        }
        Sleep(1000);
        cnt++;
    }

    // 短 uuid 走 SSO（地址即缓冲区），容量 >= 16 时 +0 处为堆指针。
    uint32_t size    = util::get_dword(strAddr + 0x10);
    uint32_t cap     = util::get_dword(strAddr + 0x14);
    const char *uuid = (cap >= 16) ? *reinterpret_cast<const char **>(strAddr)
                                   : reinterpret_cast<const char *>(strAddr);
    if (uuid == nullptr) {
        return "";
    }
    return "http://weixin.qq.com/x/" + std::string(uuid, size);
}

int receive_transfer(const std::string &wxid, const std::string &transferid, const std::string &transactionid)
{
    if (g_WeChatWinDllAddr == 0) {
        LOG_ERROR("WeChatWin.dll not located.");
        return -1;
    }

    int rv = -1;

    // WCPayInfo 带虚表且约 0x158 字节；先默认构造再逐字段 ASSIGN，最后析构。缓冲取 0x160 冗余覆盖。
    char payInfo[0x160] = { 0 };

    std::wstring wsWxid = util::s2w(wxid);
    std::wstring wsTfid = util::s2w(transferid);
    std::wstring wsTaid = util::s2w(transactionid);

    // wxid 仅被受理入口读取（内部转 std::string、真实调用者调用后自行 free），故用非拥有 WxString 视图。
    WxString wxWxid(wsWxid);

    LOG_DEBUG("Receiving transfer, from: {}, transferid: {}, transactionid: {}", wxid, transferid, transactionid);

    auto initPayInfo     = reinterpret_cast<BufferInitFn>(g_WeChatWinDllAddr + Offsets::Transfer::PAY_INFO_CTOR);
    auto receiveTransfer = reinterpret_cast<ReceiveTransferFn>(g_WeChatWinDllAddr + Offsets::Transfer::RECV_TRANSFER);
    auto assignField     = reinterpret_cast<WxStringAssignFn>(g_WeChatWinDllAddr + Offsets::RichText::ASSIGN);
    auto destroyPayInfo  = reinterpret_cast<BufferCleanupFn>(g_WeChatWinDllAddr + Offsets::Transfer::PAY_INFO_DTOR);

    // 受理入口会拷贝构造 payInfo，故所有成员须先默认构造初始化
    initPayInfo(payInfo);

    *reinterpret_cast<uint32_t *>(payInfo + Offsets::Transfer::F_PAYSUBTYPE)     = 1;
    *reinterpret_cast<uint32_t *>(payInfo + Offsets::Transfer::F_EFFECTIVE_DATE) = 1;

    // 交易号/转账单号会被析构器 mm_free，须用 ASSIGN 建 WeChat 拥有副本
    assignField(payInfo + Offsets::Transfer::F_TRANSACTION_ID, wsTaid.c_str(), -1);
    assignField(payInfo + Offsets::Transfer::F_TRANSFER_ID, wsTfid.c_str(), -1);

    // confirm=1 表示接受/领取；scratch 为废弃占位传 0。
    rv = receiveTransfer(payInfo, &wxWxid, 0, 1);
    destroyPayInfo(payInfo);

    return rv;
}

bool rpc_get_audio(const AudioMsg &am, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_AUDIO_MSG>(out, len, [&am](Response &rsp) {
        std::string path = "";
        if (am.dir == NULL) {
            LOG_ERROR("Empty dir.");
        } else {
            path = get_audio(am.id, am.dir);
        }
        rsp.msg.str = (char *)path.c_str();
    });
}

bool rpc_decrypt_image(const DecPath &dec, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_DECRYPT_IMAGE>(out, len, [&dec](Response &rsp) {
        std::string path = "";
        if ((dec.src == NULL) || (dec.dst == NULL)) {
            LOG_ERROR("Empty src or dst.");
        } else {
            path = decrypt_image(dec.src, dec.dst);
        }
        rsp.msg.str = (char *)path.c_str();
    });
}

bool rpc_refresh_pyq(uint64_t id, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_REFRESH_PYQ>(out, len, [id](Response &rsp) {
        int status     = refresh_pyq(id);
        rsp.msg.status = status;
    });
}

bool rpc_download_attachment(const AttachMsg &att, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_DOWNLOAD_ATTACH>(out, len, [&att](Response &rsp) {
        std::string thumb = att.thumb ? att.thumb : "";
        std::string extra = att.extra ? att.extra : "";
        int status        = download_attachment(att.id, thumb, extra);
        rsp.msg.status    = status;
    });
}

bool rpc_revoke_message(uint64_t id, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_REVOKE_MSG>(out, len, [id](Response &rsp) {
        int status     = revoke_message(id);
        rsp.msg.status = status;
    });
}

bool rpc_get_ocr_result(const std::string &path, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_EXEC_OCR>(out, len, [&path](Response &rsp) {
        OcrResult_t ocr    = get_ocr_result(path);
        rsp.msg.ocr.status = ocr.status;
        rsp.msg.ocr.result = (char *)ocr.result.c_str();
    });
}

bool rpc_get_login_url(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_REFRESH_QRCODE>(out, len, [](Response &rsp) {
        std::string url = get_login_url();
        rsp.msg.str     = (char *)url.c_str();
    });
}

bool rpc_receive_transfer(const Transfer &tf, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_RECV_TRANSFER>(out, len, [&tf](Response &rsp) {
        if ((tf.wxid == NULL) || (tf.tfid == NULL) || (tf.taid == NULL)) {
            LOG_ERROR("Empty wxid, tfid or taid.");
            rsp.msg.status = -1;
        } else {
            int status     = receive_transfer(tf.wxid, tf.tfid, tf.taid);
            rsp.msg.status = status;
        }
    });
}

} // namespace misc
