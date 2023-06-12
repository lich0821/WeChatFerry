#include "framework.h"
#include <sstream>

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

    WxString_t txtRoomid = { 0 };
    wstring wsRoomid     = String2Wstring(roomid);
    txtRoomid.text       = (wchar_t *)wsRoomid.c_str();
    txtRoomid.size       = wsRoomid.size();
    txtRoomid.capacity   = wsRoomid.capacity();

    vector<wstring> vMembers;
    vector<WxString_t> vTxtMembers;
    wstringstream wss(String2Wstring(wxids));
    while (wss.good()) {
        wstring wstr;
        getline(wss, wstr, L',');
        vMembers.push_back(wstr);
        WxString_t txtMember = { 0 };
        txtMember.text       = (wchar_t *)vMembers.back().c_str();
        txtMember.size       = vMembers.back().size();
        txtMember.capacity   = vMembers.back().capacity();
        vTxtMembers.push_back(txtMember);
    }

    LOG_DEBUG("Adding {} members[{}] to {}", vTxtMembers.size(), wxids.c_str(), roomid.c_str());
    __asm {
		pushad;
		pushfd;
		call addRoomMemberCall1;
		sub esp, 0x14;
		mov esi, eax;
		mov ecx, esp;
		lea eax, txtRoomid;
		push eax;
		call addRoomMemberCall2;
        lea edi, vTxtMembers
		push edi;
		mov ecx, esi;
		call addRoomMemberCall3;
		mov rv, eax;
		popfd;
		popad;
    }

    return rv;
}
