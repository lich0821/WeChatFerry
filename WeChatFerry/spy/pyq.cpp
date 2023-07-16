#include "framework.h"

#include "log.h"
#include "spy_types.h"
#include "util.h"

extern bool gIsListeningPyq;
extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

typedef struct RawVector {
    DWORD start;
    DWORD finish;
    DWORD end;
} RawVector_t;

static int GetFirstPage()
{
    int rv         = -1;
    DWORD pyqCall1 = g_WeChatWinDllAddr + 0xC39680;
    DWORD pyqCall2 = g_WeChatWinDllAddr + 0x14E2140;

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
    DWORD pyqCall1 = g_WeChatWinDllAddr + 0xC39680;
    DWORD pyqCall3 = g_WeChatWinDllAddr + 0x14E21E0;

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
