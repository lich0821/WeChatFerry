#include "framework.h"

#include "accept_new_friend.h"
#include "load_calls.h"
#include "log.h"
#include "util.h"

extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

int AcceptNewFriend(std::string v3, std::string v4, int scene)
{
    int success = 0;

    DWORD acceptNewFriendCall1 = g_WeChatWinDllAddr + g_WxCalls.anf.call1;
    DWORD acceptNewFriendCall2 = g_WeChatWinDllAddr + g_WxCalls.anf.call2;
    DWORD acceptNewFriendCall3 = g_WeChatWinDllAddr + g_WxCalls.anf.call3;
    DWORD acceptNewFriendCall4 = g_WeChatWinDllAddr + g_WxCalls.anf.call4;

    char buffer[0x40]      = { 0 };
    char nullbuffer[0x3CC] = { 0 };

    LOG_DEBUG("\nv3: {}\nv4: {}\nscene: {}", v3, v4, scene);
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
        lea ecx, buffer;
        call acceptNewFriendCall1;
        mov esi, 0x0;
        mov edi, scene;
        push esi;
        push edi;
        sub esp, 0x14;
        mov ecx, esp;
        lea eax, wxV4;
        push eax;
        call acceptNewFriendCall2;
        sub esp, 0x8;
        push 0x0;
        lea eax, nullbuffer;
        push eax;
        lea eax, wxV3;
        push eax;
        lea ecx, buffer;
        call acceptNewFriendCall3;
        mov success, eax;
        lea ecx, buffer;
        call acceptNewFriendCall4;
        popfd;
        popad;
    }

    return success; // 成功返回 1
}
