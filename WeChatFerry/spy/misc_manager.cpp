#pragma warning(disable : 4244)

#include "misc_manager.h"

#include <filesystem>
#include <fstream>

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
using BufferCleanupExFn      = void(__thiscall *)(void *buffer, int free_memory);
using WarmupFn               = void (*)();
using RefreshFirstPageFn     = int(__thiscall *)(void *manager, void *buffer, int forward);
using RefreshNextPageFn      = int(__thiscall *)(void *manager, uint32_t id_low, uint32_t id_high,
                                                 RawVector_t *cursor);
using RunOcrFn               = int(__thiscall *)(void *manager, const WxString *path, int reserved,
                                                 WxString *ocr_buffer, uint32_t *tmp, const WxString *null_obj);
using RefreshLoginQrCodeFn   = void(__thiscall *)(void *manager);
using LoadAttachmentMetaFn   = int(__thiscall *)(void *buffer, uint32_t local_id, uint32_t db_idx);
using DownloadAttachmentFn   = int(__thiscall *)(void *manager, void *buffer, int reserved, int sync);
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

    char buf[0xB44] = { 0 };
    auto getMomentsManager = reinterpret_cast<ManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Moments::CALL1);
    auto requestFirstPage  = reinterpret_cast<RefreshFirstPageFn>(g_WeChatWinDllAddr + Offsets::Moments::CALL2);

    void *manager = getMomentsManager();
    if (manager != nullptr) {
        rv = requestFirstPage(manager, buf, 1);
    }

    return rv;
}

int get_next_page(uint64_t id)
{
    int rv = -1;

    RawVector_t tmp         = { 0 };
    auto getMomentsManager  = reinterpret_cast<ManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Moments::CALL1);
    auto requestNextPage    = reinterpret_cast<RefreshNextPageFn>(g_WeChatWinDllAddr + Offsets::Moments::CALL3);
    void *manager           = getMomentsManager();
    if (manager != nullptr) {
        rv = requestNextPage(manager, static_cast<uint32_t>(id), static_cast<uint32_t>(id >> 32), &tmp);
    }

    return rv;
}

} // namespace

namespace misc
{

std::string decrypt_image(const std::string &src, const std::string &dir)
{
    (void)src;
    (void)dir;
    LOG_ERROR("Not Implemented yet.");
    return "";
}

int refresh_pyq(uint64_t id)
{
    (void)id;
    LOG_ERROR("Not Implemented yet.");
    return -1;

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
    (void)id;
    (void)thumb;
    (void)extra;
    LOG_ERROR("Not Implemented yet.");
    return -1;

    int status    = -1;
    uint64_t localId;
    uint32_t dbIdx;

    if (fs::exists(extra)) {
        return 0;
    }

    if (db::get_local_id_and_dbidx(id, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", to_string(id));
        return status;
    }

    char buff[0x2D8] = { 0 };
    auto initAttachmentBuffer = reinterpret_cast<BufferInitFn>(g_WeChatWinDllAddr + Offsets::Attachment::DL_CALL1);
    auto warmupAttachment     = reinterpret_cast<WarmupFn>(g_WeChatWinDllAddr + Offsets::Attachment::DL_CALL2);
    auto loadAttachmentMeta   = reinterpret_cast<LoadAttachmentMetaFn>(g_WeChatWinDllAddr + Offsets::Attachment::DL_CALL3);
    auto getAttachmentManager = reinterpret_cast<ManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Attachment::DL_CALL4);
    auto startAttachmentDl    = reinterpret_cast<DownloadAttachmentFn>(g_WeChatWinDllAddr + Offsets::Attachment::DL_CALL5);
    auto cleanupAttachment    = reinterpret_cast<BufferCleanupExFn>(g_WeChatWinDllAddr + Offsets::Attachment::DL_CALL6);

    initAttachmentBuffer(buff);
    warmupAttachment();
    loadAttachmentMeta(buff, static_cast<uint32_t>(localId), dbIdx);

    uint32_t type = util::get_dword((uint32_t)(buff + 0x38));

    std::string save_path  = "";
    std::string thumb_path = "";

    switch (type) {
        case 0x03:
            save_path = extra;
            break;
        case 0x3E:
        case 0x2B:
            thumb_path = thumb;
            save_path  = fs::path(thumb).replace_extension("mp4").string();
            break;
        case 0x31:
            save_path = extra;
            break;
        default:
            break;
    }

    if (fs::exists(save_path)) {
        return 0;
    }

    LOG_DEBUG("path: {}", save_path);
    fs::create_directory(fs::path(save_path).parent_path().string());

    std::wstring wsSavePath  = util::s2w(save_path);
    std::wstring wsThumbPath = util::s2w(thumb_path);

    WxString wxSavePath(wsSavePath);
    WxString wxThumbPath(wsThumbPath);

    int temp = 1;
    memcpy(&buff[0x19C], &wxThumbPath, sizeof(wxThumbPath));
    memcpy(&buff[0x1B0], &wxSavePath, sizeof(wxSavePath));
    memcpy(&buff[0x29C], &temp, sizeof(temp));

    void *manager = getAttachmentManager();
    if (manager != nullptr) {
        status = startAttachmentDl(manager, buff, 0, 1);
    }
    cleanupAttachment(buff, 0);

    return status;
}

int revoke_message(uint64_t id)
{
    (void)id;
    LOG_ERROR("Not Implemented yet.");
    return -1;

    int status    = -1;
    uint64_t localId;
    uint32_t dbIdx;
    if (db::get_local_id_and_dbidx(id, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", to_string(id));
        return status;
    }

    char chat_msg[0x2D8] = { 0 };

    auto initRevokeBuffer   = reinterpret_cast<BufferInitFn>(g_WeChatWinDllAddr + Offsets::Revoke::CALL1);
    auto getRevokeManager   = reinterpret_cast<ManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Revoke::CALL2);
    auto loadRevokeMeta     = reinterpret_cast<LoadAttachmentMetaFn>(g_WeChatWinDllAddr + Offsets::Revoke::CALL3);
    auto revokeMessageFn    = reinterpret_cast<RevokeMessageFn>(g_WeChatWinDllAddr + Offsets::Revoke::CALL4);
    auto cleanupRevoke      = reinterpret_cast<BufferCleanupExFn>(g_WeChatWinDllAddr + Offsets::Revoke::CALL5);

    initRevokeBuffer(chat_msg);
    getRevokeManager();
    loadRevokeMeta(chat_msg, static_cast<uint32_t>(localId), dbIdx);

    void *manager = getRevokeManager();
    if (manager != nullptr) {
        status = revokeMessageFn(manager, chat_msg);
    }
    cleanupRevoke(chat_msg, 0);

    return status;
}

std::string get_audio(uint64_t id, const std::string &dir)
{
    (void)id;
    (void)dir;
    LOG_ERROR("Not Implemented yet.");
    return "";

    std::string mp3path = (dir.back() == '\\' || dir.back() == '/') ? dir : (dir + "/");
    mp3path += to_string(id) + ".mp3";
    replace(mp3path.begin(), mp3path.end(), '\\', '/');
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

OcrResult_t get_ocr_result(const std::string &path)
{
    (void)path;
    LOG_ERROR("Not Implemented yet.");
    return { -1, "" };

    OcrResult_t ret = { -1, "" };

    if (!fs::exists(path)) {
        LOG_ERROR("Can not find: {}", path);
        return ret;
    }

    std::wstring wsPath = util::s2w(fs::path(path).make_preferred().string());

    WxString wxPath(wsPath);
    WxString nullObj;
    WxString ocrBuffer;

    auto initOcrBuffer   = reinterpret_cast<BufferInitFn>(g_WeChatWinDllAddr + Offsets::OCR::CALL1);
    auto getOcrManager   = reinterpret_cast<ManagerGetterFn>(g_WeChatWinDllAddr + Offsets::OCR::CALL2);
    auto runOcr          = reinterpret_cast<RunOcrFn>(g_WeChatWinDllAddr + Offsets::OCR::CALL3);

    uint32_t tmp = 0;
    int status   = -1;
    initOcrBuffer(&ocrBuffer);

    void *manager = getOcrManager();
    if (manager != nullptr) {
        status = runOcr(manager, &wxPath, 0, &ocrBuffer, &tmp, &nullObj);
    }

    if (status != 0) {
        LOG_ERROR("OCR status: {}", to_string(status));
        return ret;
    }

    ret.status = status;

    uint32_t addr   = (DWORD)&ocrBuffer;
    uint32_t header = util::get_dword(addr);
    uint32_t num    = util::get_dword(addr + 0x4);
    if (num <= 0) {
        return ret;
    }

    for (uint32_t i = 0; i < num; i++) {
        uint32_t content = util::get_dword(header);
        ret.result += util::w2s(util::get_wstring(content + 0x14));
        ret.result += "\n";
        header = content;
    }

    return ret;
}

std::string get_login_url()
{
    LOG_ERROR("Not Implemented yet.");
    return "";

    if (util::get_dword(g_WeChatWinDllAddr + Offsets::Account::SERVICE) == 1) {
        LOG_DEBUG("Already logined.");
        return "";
    }

    auto getQrCodeManager = reinterpret_cast<ManagerGetterFn>(g_WeChatWinDllAddr + Offsets::QRCode::CALL1);
    auto refreshQrCode    = reinterpret_cast<RefreshLoginQrCodeFn>(g_WeChatWinDllAddr + Offsets::QRCode::CALL2);

    void *manager = getQrCodeManager();
    if (manager != nullptr) {
        refreshQrCode(manager);
    }

    const char *url = util::get_string(g_WeChatWinDllAddr + Offsets::QRCode::URL);
    uint8_t cnt     = 0;
    while (url[0] == 0) {
        if (cnt > 5) {
            LOG_ERROR("Refresh QR Code timeout.");
            return "";
        }
        Sleep(1000);
        cnt++;
    }
    return "http://weixin.qq.com/x/" + std::string(url);
}

int receive_transfer(const std::string &wxid, const std::string &transferid, const std::string &transactionid)
{
    (void)wxid;
    (void)transferid;
    (void)transactionid;
    LOG_ERROR("Not Implemented yet.");
    return -1;

    int rv                = 0;
    char payInfo[0x134] = { 0 };
    std::wstring wsWxid = util::s2w(wxid);
    std::wstring wsTfid = util::s2w(transferid);
    std::wstring wsTaid = util::s2w(transactionid);

    WxString wxWxid(wsWxid);
    WxString wxTfid(wsTfid);
    WxString wxTaid(wsTaid);

    LOG_DEBUG("Receiving transfer, from: {}, transferid: {}, transactionid: {}", wxid, transferid, transactionid);
    auto initTransferInfo = reinterpret_cast<BufferInitFn>(g_WeChatWinDllAddr + Offsets::Transfer::CALL1);
    auto receiveTransfer  = reinterpret_cast<ReceiveTransferFn>(g_WeChatWinDllAddr + Offsets::Transfer::CALL2);
    auto cleanupTransfer  = reinterpret_cast<BufferCleanupExFn>(g_WeChatWinDllAddr + Offsets::Transfer::CALL3);

    initTransferInfo(payInfo);
    *reinterpret_cast<uint32_t *>(payInfo + 0x4)  = 0x1;
    *reinterpret_cast<uint32_t *>(payInfo + 0x4C) = 0x1;
    memcpy(&payInfo[0x1C], &wxTaid, sizeof(wxTaid));
    memcpy(&payInfo[0x38], &wxTfid, sizeof(wxTfid));

    rv = receiveTransfer(payInfo, &wxWxid, 0, 1);
    cleanupTransfer(payInfo, 0);

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
