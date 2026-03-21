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
    int rv           = -1;
    uint32_t pyqCall1 = g_WeChatWinDllAddr + Offsets::Moments::CALL1;
    uint32_t pyqCall2 = g_WeChatWinDllAddr + Offsets::Moments::CALL2;

    char buf[0xB44] = { 0 };
    __asm {
        pushad;
        call pyqCall1;
        push 0x1;
        lea ecx, buf;
        push ecx;
        mov ecx, eax;
        call pyqCall2;
        mov rv, eax;
        popad;
    }

    return rv;
}

int get_next_page(uint64_t id)
{
    int rv           = -1;
    uint32_t pyqCall1 = g_WeChatWinDllAddr + Offsets::Moments::CALL1;
    uint32_t pyqCall3 = g_WeChatWinDllAddr + Offsets::Moments::CALL3;

    RawVector_t tmp = { 0 };

    __asm {
        pushad;
        call pyqCall1;
        lea ecx, tmp;
        push ecx;
        mov ebx, dword ptr [id + 0x04];
        push ebx;
        mov edi, dword ptr [id]
        push edi;
        mov ecx, eax;
        call pyqCall3;
        mov rv, eax;
        popad;
    }

    return rv;
}

} // namespace

namespace misc
{

std::string decrypt_image(const std::string &src, const std::string &dir)
{
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

    uint8_t key    = 0x00;
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

        replace(dst.begin(), dst.end(), '\\', '/');
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
    uint32_t dlCall1 = g_WeChatWinDllAddr + Offsets::Attachment::DL_CALL1;
    uint32_t dlCall2 = g_WeChatWinDllAddr + Offsets::Attachment::DL_CALL2;
    uint32_t dlCall3 = g_WeChatWinDllAddr + Offsets::Attachment::DL_CALL3;
    uint32_t dlCall4 = g_WeChatWinDllAddr + Offsets::Attachment::DL_CALL4;
    uint32_t dlCall5 = g_WeChatWinDllAddr + Offsets::Attachment::DL_CALL5;
    uint32_t dlCall6 = g_WeChatWinDllAddr + Offsets::Attachment::DL_CALL6;

    __asm {
        pushad;
        pushfd;
        lea ecx, buff;
        call dlCall1;
        call dlCall2;
        push dword ptr [dbIdx];
        lea ecx, buff;
        push dword ptr [localId];
        call dlCall3;
        add esp, 0x8;
        popfd;
        popad;
    }

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

    __asm {
        pushad;
        pushfd;
        call dlCall4;
        push 0x1;
        push 0x0;
        lea ecx, buff;
        push ecx;
        mov ecx, eax;
        call dlCall5;
        mov status, eax;
        lea ecx, buff;
        push 0x0;
        call dlCall6;
        popfd;
        popad;
    }

    return status;
}

int revoke_message(uint64_t id)
{
    int status    = -1;
    uint64_t localId;
    uint32_t dbIdx;
    if (db::get_local_id_and_dbidx(id, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", to_string(id));
        return status;
    }

    char chat_msg[0x2D8] = { 0 };

    uint32_t rmCall1 = g_WeChatWinDllAddr + Offsets::Revoke::CALL1;
    uint32_t rmCall2 = g_WeChatWinDllAddr + Offsets::Revoke::CALL2;
    uint32_t rmCall3 = g_WeChatWinDllAddr + Offsets::Revoke::CALL3;
    uint32_t rmCall4 = g_WeChatWinDllAddr + Offsets::Revoke::CALL4;
    uint32_t rmCall5 = g_WeChatWinDllAddr + Offsets::Revoke::CALL5;

    __asm {
        pushad;
        pushfd;
        lea        ecx, chat_msg;
        call       rmCall1;
        call       rmCall2;
        push       dword ptr [dbIdx];
        lea        ecx, chat_msg;
        push       dword ptr [localId];
        call       rmCall3;
        add        esp, 0x8;
        call       rmCall2;
        lea        ecx, chat_msg;
        push       ecx;
        mov        ecx, eax;
        call       rmCall4;
        mov        status, eax;
        lea        ecx, chat_msg;
        push       0x0;
        call       rmCall5;
        popfd;
        popad;
    }

    return status;
}

std::string get_audio(uint64_t id, const std::string &dir)
{
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
    OcrResult_t ret = { -1, "" };

    if (!fs::exists(path)) {
        LOG_ERROR("Can not find: {}", path);
        return ret;
    }

    std::wstring wsPath = util::s2w(fs::path(path).make_preferred().string());

    WxString wxPath(wsPath);
    WxString nullObj;
    WxString ocrBuffer;

    uint32_t ocrCall1 = g_WeChatWinDllAddr + Offsets::OCR::CALL1;
    uint32_t ocrCall2 = g_WeChatWinDllAddr + Offsets::OCR::CALL2;
    uint32_t ocrCall3 = g_WeChatWinDllAddr + Offsets::OCR::CALL3;

    uint32_t tmp = 0;
    int status   = -1;
    __asm {
        pushad;
        pushfd;
        lea   ecx, ocrBuffer;
        call  ocrCall1;
        call  ocrCall2;
        lea   ecx, nullObj;
        push  ecx;
        lea   ecx, tmp;
        push  ecx;
        lea   ecx, ocrBuffer;
        push  ecx;
        push  0x0;
        lea   ecx, wxPath;
        push  ecx;
        mov   ecx, eax;
        call  ocrCall3;
        mov   status, eax;
        popfd;
        popad;
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
    if (util::get_dword(g_WeChatWinDllAddr + Offsets::Account::SERVICE) == 1) {
        LOG_DEBUG("Already logined.");
        return "";
    }

    uint32_t refreshLoginQrcodeCall1 = g_WeChatWinDllAddr + Offsets::QRCode::CALL1;
    uint32_t refreshLoginQrcodeCall2 = g_WeChatWinDllAddr + Offsets::QRCode::CALL2;

    __asm {
        pushad;
        pushfd;
        call refreshLoginQrcodeCall1;
        mov ecx, eax;
        call refreshLoginQrcodeCall2;
        popfd;
        popad;
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
    int rv                = 0;
    uint32_t call1        = g_WeChatWinDllAddr + Offsets::Transfer::CALL1;
    uint32_t call2        = g_WeChatWinDllAddr + Offsets::Transfer::CALL2;
    uint32_t call3        = g_WeChatWinDllAddr + Offsets::Transfer::CALL3;

    char payInfo[0x134] = { 0 };
    std::wstring wsWxid = util::s2w(wxid);
    std::wstring wsTfid = util::s2w(transferid);
    std::wstring wsTaid = util::s2w(transactionid);

    WxString wxWxid(wsWxid);
    WxString wxTfid(wsTfid);
    WxString wxTaid(wsTaid);

    LOG_DEBUG("Receiving transfer, from: {}, transferid: {}, transactionid: {}", wxid, transferid, transactionid);
    __asm {
        pushad;
        lea ecx, payInfo;
        call call1;
        mov dword ptr[payInfo + 0x4], 0x1;
        mov dword ptr[payInfo + 0x4C], 0x1;
        popad;
    }
    memcpy(&payInfo[0x1C], &wxTaid, sizeof(wxTaid));
    memcpy(&payInfo[0x38], &wxTfid, sizeof(wxTfid));

    __asm {
        pushad;
        push 0x1;
        sub esp, 0x8;
        lea edx, wxWxid;
        lea ecx, payInfo;
        call call2;
        mov rv, eax;
        add esp, 0xC;
        push 0x0;
        lea ecx, payInfo;
        call call3;
        popad;
    }

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
