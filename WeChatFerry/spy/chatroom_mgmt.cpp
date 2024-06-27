#include "framework.h"
#include <sstream>
#include <vector>

#include "chatroom_mgmt.h"
#include "load_calls.h"
#include "log.h"
#include "util.h"

using namespace std;

extern WxCalls_t g_WxCalls;
extern QWORD g_WeChatWinDllAddr;

typedef QWORD (*funcGetChatRoomMgr_t)();
typedef QWORD (*funcAddMemberToChatRoom_t)(QWORD, QWORD, QWORD, QWORD);
typedef QWORD (*funcDelMemberFromChatRoom_t)(QWORD, QWORD, QWORD);
typedef QWORD (*funcInviteMemberToChatRoom_t)(QWORD, QWORD, QWORD, QWORD);

int AddChatroomMember(string roomid, string wxids)
{
    int status = -1;

    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return status;
    }

    funcGetChatRoomMgr_t GetChatRoomMgr  = (funcGetChatRoomMgr_t)(g_WeChatWinDllAddr + g_WxCalls.arm.call1);
    funcAddMemberToChatRoom_t AddMembers = (funcAddMemberToChatRoom_t)(g_WeChatWinDllAddr + g_WxCalls.arm.call2);

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

    QWORD temp[2]       = { 0 };
    WxString *pWxRoomid = NewWxStringFromStr(roomid);
    QWORD pMembers      = (QWORD) & ((RawVector_t *)&vWxMembers)->start;

    QWORD mgr = GetChatRoomMgr();
    status    = (int)AddMembers(mgr, pMembers, (QWORD)pWxRoomid, (QWORD)temp);
    return status;
}

int DelChatroomMember(string roomid, string wxids)
{
    int status = -1;

    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return status;
    }

    funcGetChatRoomMgr_t GetChatRoomMgr    = (funcGetChatRoomMgr_t)(g_WeChatWinDllAddr + g_WxCalls.drm.call1);
    funcDelMemberFromChatRoom_t DelMembers = (funcDelMemberFromChatRoom_t)(g_WeChatWinDllAddr + g_WxCalls.drm.call2);

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

    WxString *pWxRoomid = NewWxStringFromStr(roomid);
    QWORD pMembers      = (QWORD) & ((RawVector_t *)&vWxMembers)->start;

    QWORD mgr = GetChatRoomMgr();
    status    = (int)DelMembers(mgr, pMembers, (QWORD)pWxRoomid);
    return status;
}

int InviteChatroomMember(string roomid, string wxids)
{
    int status = -1;

    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return status;
    }

    funcInviteMemberToChatRoom_t InviteMembers
        = (funcInviteMemberToChatRoom_t)(g_WeChatWinDllAddr + g_WxCalls.irm.call1);

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
    QWORD temp[2]       = { 0 };
    wstring wsRoomid    = String2Wstring(roomid);
    WxString *pWxRoomid = NewWxStringFromWstr(wsRoomid);
    QWORD pMembers      = (QWORD) & ((RawVector_t *)&vWxMembers)->start;

    status = (int)InviteMembers((QWORD)wsRoomid.c_str(), pMembers, (QWORD)pWxRoomid, (QWORD)temp);
    return status;
}
