#include "accept_new_friend.h"
#include "load_calls.h"

typedef struct NewFriendParam {
    DWORD handle;
    DWORD *status;
    DWORD statusEnd1;
    DWORD statusEnd2;
    char buffer[0x3C];
} NewFriendParam_t;

extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

BOOL AcceptNewFriend(std::wstring v3, std::wstring v4)
{
    BOOL isSucceeded = false;

    DWORD acceptNewFriendCall1  = g_WeChatWinDllAddr + g_WxCalls.anf.call1;
    DWORD acceptNewFriendCall2  = g_WeChatWinDllAddr + g_WxCalls.anf.call2;
    DWORD acceptNewFriendHandle = g_WeChatWinDllAddr + g_WxCalls.anf.handle;

    char buffer[0x94]      = { 0 };
    NewFriendParam_t param = { 0 };
    DWORD status[9] = { 0xB2, (DWORD)&param, 0xB5, (DWORD)&param, 0xB0, (DWORD)&param, 0xB1, (DWORD)&param, 0x00 };

    param.handle             = acceptNewFriendHandle;
    param.status             = status;
    param.statusEnd1         = (DWORD)&status[8];
    param.statusEnd2         = (DWORD)&status[8];
    NewFriendParam_t *pParam = &param;

    __asm {
        pushad;
        pushfd;
        push 0x0;
        push 0x6;
        sub esp, 0x14;
        mov ecx, esp;
        lea eax, v4;
        push eax;
        call acceptNewFriendCall1;
        sub esp, 0x8;
        push 0x0;
        lea eax, buffer;
        push eax;
        lea eax, v3;
        push eax;
        mov ecx, pParam;
        call acceptNewFriendCall2;
        mov isSucceeded, eax;
        popfd;
        popad;
    }

    return isSucceeded;
}
