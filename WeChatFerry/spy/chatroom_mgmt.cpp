#include "framework.h"
#include <sstream>
#include <vector>

#include "chatroom_mgmt.h"
#include "load_calls.h"
#include "log.h"
#include "util.h"

using namespace std;

extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

int AddChatroomMember(string roomid, string wxids)
{
    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return -1;
    }

    int rv                   = 0;
    DWORD addRoomMemberCall1 = g_WeChatWinDllAddr + g_WxCalls.arm.call1;
    DWORD addRoomMemberCall2 = g_WeChatWinDllAddr + g_WxCalls.arm.call2;
    DWORD addRoomMemberCall3 = g_WeChatWinDllAddr + g_WxCalls.arm.call3;

    DWORD temp       = 0;
    wstring wsRoomid = String2Wstring(roomid);
    WxString txtRoomid(wsRoomid);

    vector<wstring> vMembers;
    vector<WxString> vTxtMembers;
    wstringstream wss(String2Wstring(wxids));
    while (wss.good()) {
        wstring wstr;
        getline(wss, wstr, L',');
        vMembers.push_back(wstr);
        WxString txtMember(vMembers.back());
        vTxtMembers.push_back(txtMember);
    }

    LOG_DEBUG("Adding {} members[{}] to {}", vTxtMembers.size(), wxids.c_str(), roomid.c_str());
    __asm {
        pushad;
        pushfd;
        call addRoomMemberCall1;
        sub esp, 0x8;
        mov temp, eax;
        mov ecx, esp;
        mov dword ptr[ecx], 0x0;
        mov dword ptr[ecx + 4], 0x0;
        test esi, esi;
        sub esp, 0x14;
        mov ecx, esp;
        lea eax, txtRoomid;
        push eax;
        call addRoomMemberCall2;
        mov ecx, temp;
        lea eax, vTxtMembers;
        push eax;
        call addRoomMemberCall3;
        mov rv, eax;
        popfd;
        popad;
    }
    return rv;
}

int DelChatroomMember(string roomid, string wxids)
{
    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return -1;
    }

    int rv                   = 0;
    DWORD delRoomMemberCall1 = g_WeChatWinDllAddr + g_WxCalls.drm.call1;
    DWORD delRoomMemberCall2 = g_WeChatWinDllAddr + g_WxCalls.drm.call2;
    DWORD delRoomMemberCall3 = g_WeChatWinDllAddr + g_WxCalls.drm.call3;

    DWORD temp       = 0;
    wstring wsRoomid = String2Wstring(roomid);
    WxString txtRoomid(wsRoomid);

    vector<wstring> vMembers;
    vector<WxString> vTxtMembers;
    wstringstream wss(String2Wstring(wxids));
    while (wss.good()) {
        wstring wstr;
        getline(wss, wstr, L',');
        vMembers.push_back(wstr);
        WxString txtMember(vMembers.back());
        vTxtMembers.push_back(txtMember);
    }

    LOG_DEBUG("Adding {} members[{}] to {}", vTxtMembers.size(), wxids.c_str(), roomid.c_str());
    __asm {
        pushad;
        pushfd;
        call delRoomMemberCall1;
        sub esp, 0x14;
        mov esi, eax;
        mov ecx, esp;
        lea edi, txtRoomid;
        push edi;
        call delRoomMemberCall2;
        mov ecx, esi;
        lea eax, vTxtMembers;
        push eax;
        call delRoomMemberCall3;
        mov rv, eax;
        popfd;
        popad;
    }
    return rv;
}

int InviteChatroomMember(string roomid, string wxids)
{
    wstring wsRoomid = String2Wstring((roomid));
    WxString wxRoomid(wsRoomid);

    vector<wstring> vMembers;
    vector<WxString> vWxMembers;
    wstringstream wss(String2Wstring(wxids));
    while (wss.good()) {
        wstring wstr;
        getline(wss, wstr, L',');
        vMembers.push_back(wstr);
        WxString wxMember(vMembers.back());
        vWxMembers.push_back(wxMember);
    }

    LOG_DEBUG("Inviting {} members[{}] to {}", vWxMembers.size(), wxids.c_str(), roomid.c_str());

    DWORD irmCall1 = g_WeChatWinDllAddr + g_WxCalls.irm.call1;
    DWORD irmCall2 = g_WeChatWinDllAddr + g_WxCalls.irm.call2;
    DWORD irmCall3 = g_WeChatWinDllAddr + g_WxCalls.irm.call3;
    DWORD irmCall4 = g_WeChatWinDllAddr + g_WxCalls.irm.call4;
    DWORD irmCall5 = g_WeChatWinDllAddr + g_WxCalls.irm.call5;
    DWORD irmCall6 = g_WeChatWinDllAddr + g_WxCalls.irm.call6;
    DWORD irmCall7 = g_WeChatWinDllAddr + g_WxCalls.irm.call7;
    DWORD irmCall8 = g_WeChatWinDllAddr + g_WxCalls.irm.call8;

    DWORD sys_addr = (DWORD)GetModuleHandleA("win32u.dll") + 0x116C;
    DWORD addr[2]  = { sys_addr, 0 };
    __asm {
        pushad;
        pushfd;
        call irmCall1;
        lea  ecx, addr;
        push ecx;
        mov  ecx, eax;
        call irmCall2;
        call irmCall3;
        sub  esp, 0x8;
        lea  eax, addr;
        mov  ecx, esp;
        push eax;
        call irmCall4;
        sub  esp, 0x14;
        mov  ecx, esp;
        lea  eax, wxRoomid;
        push eax;
        call irmCall5;
        lea  eax, vWxMembers;
        push eax;
        call irmCall6;
        call irmCall1;
        push 0x0;
        push 0x1;
        mov  ecx, eax;
        call irmCall7;
        lea  ecx, addr;
        call irmCall8;
        popfd;
        popad;
    }
    return 1;
}
