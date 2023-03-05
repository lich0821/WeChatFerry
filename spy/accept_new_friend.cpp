#include "framework.h"

#include "accept_new_friend.h"
#include "load_calls.h"
#include "log.h"
#include "util.h"

typedef struct NewFriendParam {
    DWORD handle;
    DWORD *status;
    DWORD statusEnd1;
    DWORD statusEnd2;
    char buffer[0x3C];
} NewFriendParam_t;

extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

int AcceptNewFriend(std::string v3, std::string v4)
{
    int isSucceeded = 0;

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

    LOG_DEBUG("v3: {}\nv4: {}", v3, v4);
    WxString_t wxV3   = { 0 };
    WxString_t wxV4   = { 0 };
    std::wstring wsV3 = String2Wstring(v3);
    std::wstring wsV4 = String2Wstring(v4);

    wxV3.text     = (wchar_t *)wsV3.c_str();
    wxV3.size     = wsV3.size();
    wxV3.capacity = wsV3.capacity();

    wxV4.text     = (wchar_t *)wsV4.c_str();
    wxV4.size     = wsV4.size();
    wxV4.capacity = wsV4.capacity();

    __asm {
        pushad;
        pushfd;
        push 0x0;
        push 0x6;
        sub esp, 0x14;
        mov ecx, esp;
        lea eax, wxV4;
        push eax;
        call acceptNewFriendCall1;
        sub esp, 0x8;
        push 0x0;
        lea eax, buffer;
        push eax;
        lea eax, wxV3;
        push eax;
        mov ecx, pParam;
        call acceptNewFriendCall2;
        mov isSucceeded, eax;
        popfd;
        popad;
    }

    return isSucceeded; // 成功返回 1
}
