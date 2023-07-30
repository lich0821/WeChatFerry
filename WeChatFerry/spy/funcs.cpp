#pragma warning(disable : 4244)

#include "framework.h"
#include <fstream>

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

extern bool gIsListeningPyq;
extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

typedef struct RawVector {
    DWORD start;
    DWORD finish;
    DWORD end;
} RawVector_t;

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

bool DecryptImage(string src, string dst)
{
    ifstream in(src.c_str(), ios::binary);
    if (!in.is_open()) {
        LOG_ERROR("Failed to open file {}", src);
        return false;
    }

    filebuf *pfb = in.rdbuf();
    size_t size  = pfb->pubseekoff(0, ios::end, ios::in);
    pfb->pubseekpos(0, ios::in);

    char *pBuf = new char[size];
    pfb->sgetn(pBuf, size);
    in.close();

    uint8_t key = 0x00;
    string ext  = get_key(pBuf[0], pBuf[1], &key);
    if (ext.empty()) {
        LOG_ERROR("Failed to get key.");
        return false;
    }

    for (size_t i = 0; i < size; i++) {
        pBuf[i] ^= key;
    }

    ofstream out((dst + ext).c_str(), ios::binary);
    if (!out.is_open()) {
        LOG_ERROR("Failed to open file {}", dst);
        return false;
    }

    out.write(pBuf, size);
    out.close();

    delete[] pBuf;

    return true;
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
