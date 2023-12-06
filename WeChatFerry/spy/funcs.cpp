#pragma warning(disable : 4244)

#include "framework.h"
#include <filesystem>
#include <fstream>

#include "codec.h"
#include "exec_sql.h"
#include "funcs.h"
#include "log.h"
#include "spy_types.h"
#include "util.h"

#define HEADER_PNG1 0x89
#define HEADER_PNG2 0x50
#define HEADER_JPG1 0xFF
#define HEADER_JPG2 0xD8
#define HEADER_GIF1 0x47
#define HEADER_GIF2 0x49

using namespace std;
namespace fs = std::filesystem;

extern bool gIsListeningPyq;
extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

static string get_key(uint8_t header1, uint8_t header2, uint8_t *key)
{
    // PNG?
    *key = HEADER_PNG1 ^ header1;
    if ((HEADER_PNG2 ^ *key) == header2) {
        return ".png";
    }

    // JPG?
    *key = HEADER_JPG1 ^ header1;
    if ((HEADER_JPG2 ^ *key) == header2) {
        return ".jpg";
    }

    // GIF?
    *key = HEADER_GIF1 ^ header1;
    if ((HEADER_GIF2 ^ *key) == header2) {
        return ".gif";
    }

    return ""; // 错误
}

string DecryptImage(string src, string dir)
{
    if (!fs::exists(src)) {
        return "";
    }

    ifstream in(src.c_str(), ios::binary);
    if (!in.is_open()) {
        LOG_ERROR("Failed to read file {}", src);
        return "";
    }

    filebuf *pfb = in.rdbuf();
    size_t size  = pfb->pubseekoff(0, ios::end, ios::in);
    pfb->pubseekpos(0, ios::in);

    vector<char> buff;
    buff.reserve(size);
    char *pBuf = buff.data();
    pfb->sgetn(pBuf, size);
    in.close();

    uint8_t key = 0x00;
    string ext  = get_key(pBuf[0], pBuf[1], &key);
    if (ext.empty()) {
        LOG_ERROR("Failed to get key.");
        return "";
    }

    for (size_t i = 0; i < size; i++) {
        pBuf[i] ^= key;
    }

    string dst = "";

    try {
        if (dir.empty()) {
            dst = fs::path(src).replace_extension(ext).string();
        } else {
            dst = (dir.back() == '\\' || dir.back() == '/') ? dir : (dir + "/");
            dst += fs::path(src).stem().string() + ext;
        }

        replace(dst.begin(), dst.end(), '\\', '/');
    } catch (const std::exception &e) {
        LOG_ERROR(GB2312ToUtf8(e.what()));
    } catch (...) {
        LOG_ERROR("Unknow exception.");
        return "";
    }

    ofstream out(dst.c_str(), ios::binary);
    if (!out.is_open()) {
        LOG_ERROR("Failed to write file {}", dst);
        return "";
    }

    out.write(pBuf, size);
    out.close();

    return dst;
}

static int GetFirstPage()
{
    int rv         = -1;
    DWORD pyqCall1 = g_WeChatWinDllAddr + g_WxCalls.pyq.call1;
    DWORD pyqCall2 = g_WeChatWinDllAddr + g_WxCalls.pyq.call2;

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

static int GetNextPage(uint64_t id)
{
    int rv         = -1;
    DWORD pyqCall1 = g_WeChatWinDllAddr + g_WxCalls.pyq.call1;
    DWORD pyqCall3 = g_WeChatWinDllAddr + g_WxCalls.pyq.call3;

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

int RefreshPyq(uint64_t id)
{
    if (!gIsListeningPyq) {
        LOG_ERROR("没有启动朋友圈消息接收，参考：enable_receiving_msg");
        return -1;
    }

    if (id == 0) {
        return GetFirstPage();
    }

    return GetNextPage(id);
}

int DownloadAttach(uint64_t id, string thumb, string extra)
{
    int status = -1;
    uint64_t localId;
    uint32_t dbIdx;

    if (fs::exists(extra)) { // 第一道，不重复下载
        return 0;
    }

    if (GetLocalIdandDbidx(id, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", to_string(id));
        return status;
    }

    char buff[0x2D8] = { 0 };
    DWORD dlCall1    = g_WeChatWinDllAddr + g_WxCalls.da.call1;
    DWORD dlCall2    = g_WeChatWinDllAddr + g_WxCalls.da.call2;
    DWORD dlCall3    = g_WeChatWinDllAddr + g_WxCalls.da.call3;
    DWORD dlCall4    = g_WeChatWinDllAddr + g_WxCalls.da.call4;
    DWORD dlCall5    = g_WeChatWinDllAddr + g_WxCalls.da.call5;
    DWORD dlCall6    = g_WeChatWinDllAddr + g_WxCalls.da.call6;

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

    DWORD type = GET_DWORD(buff + 0x38);

    string save_path  = "";
    string thumb_path = "";

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
            break;
    }

    if (fs::exists(save_path)) { // 不重复下载
        return 0;
    }

    LOG_DEBUG("path: {}", save_path);
    // 创建父目录，由于路径来源于微信，不做检查
    fs::create_directory(fs::path(save_path).parent_path().string());

    wstring wsSavePath  = String2Wstring(save_path);
    wstring wsThumbPath = String2Wstring(thumb_path);

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

int RevokeMsg(uint64_t id)
{
    int status = -1;
    uint64_t localId;
    uint32_t dbIdx;
    if (GetLocalIdandDbidx(id, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", to_string(id));
        return status;
    }

    char chat_msg[0x2D8] = { 0 };

    DWORD rmCall1 = g_WeChatWinDllAddr + g_WxCalls.rm.call1;
    DWORD rmCall2 = g_WeChatWinDllAddr + g_WxCalls.rm.call2;
    DWORD rmCall3 = g_WeChatWinDllAddr + g_WxCalls.rm.call3;
    DWORD rmCall4 = g_WeChatWinDllAddr + g_WxCalls.rm.call4;
    DWORD rmCall5 = g_WeChatWinDllAddr + g_WxCalls.rm.call5;

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

string GetAudio(uint64_t id, string dir)
{
    string mp3path = (dir.back() == '\\' || dir.back() == '/') ? dir : (dir + "/");
    mp3path += to_string(id) + ".mp3";
    replace(mp3path.begin(), mp3path.end(), '\\', '/');
    if (fs::exists(mp3path)) { // 不重复下载
        return mp3path;
    }

    vector<uint8_t> silk = GetAudioData(id);
    if (silk.size() == 0) {
        LOG_ERROR("Empty audio data.");
        return "";
    }

    Silk2Mp3(silk, mp3path, 24000);

    return mp3path;
}

OcrResult_t GetOcrResult(string path)
{
    OcrResult_t ret = { -1, "" };

    if (!fs::exists(path)) {
        LOG_ERROR("Can not find: {}", path);
        return ret;
    }

    // 路径分隔符有要求，必须为 `\`
    wstring wsPath = String2Wstring(fs::path(path).make_preferred().string());

    WxString wxPath(wsPath);
    WxString nullObj;
    WxString ocrBuffer;

    DWORD ocrCall1 = g_WeChatWinDllAddr + g_WxCalls.ocr.call1;
    DWORD ocrCall2 = g_WeChatWinDllAddr + g_WxCalls.ocr.call2;
    DWORD ocrCall3 = g_WeChatWinDllAddr + g_WxCalls.ocr.call3;

    DWORD tmp  = 0;
    int status = -1;
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

    if (status != 0)
    {
        LOG_ERROR("OCR status: {}", to_string(status));
        return ret; // 识别出错
    }

    ret.status = status;

    DWORD addr   = (DWORD)&ocrBuffer;
    DWORD header = GET_DWORD(addr);
    DWORD num    = GET_DWORD(addr + 0x4);
    if (num <= 0) {
        return ret; // 识别内容为空
    }

    for (uint32_t i = 0; i < num; i++) {
        DWORD content = GET_DWORD(header);
        ret.result += Wstring2String(GET_WSTRING(content + 0x14));
        ret.result += "\n";
        header = content;
    }

    return ret;
}
