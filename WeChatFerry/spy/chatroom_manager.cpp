#pragma execution_character_set("utf-8")

#include "chatroom_manager.h"
#include "log.hpp"
#include "pb_util.h"
#include "rpc_helper.h"
#include "util.h"

using namespace std;
extern QWORD g_WeChatWinDllAddr;

namespace chatroom
{
#define OS_GET_CHATROOM_MGR 0x1B83BD0
#define OS_ADD_MEMBERS      0x2155100
#define OS_DELETE_MEMBERS   0x2155740
#define OS_INVITE_MEMBERS   0x2154AE0

using get_chatroom_mgr_t          = QWORD (*)();
using add_member_to_chatroom_t    = QWORD (*)(QWORD, QWORD, QWORD, QWORD);
using del_member_from_chatroom_t  = QWORD (*)(QWORD, QWORD, QWORD);
using invite_member_to_chatroom_t = QWORD (*)(QWORD, QWORD, QWORD, QWORD);

static vector<WxString> parse_wxids(const string &wxids)
{
    vector<WxString> wx_members;
    wstringstream wss(util::s2w(wxids));
    wstring wstr;
    while (getline(wss, wstr, L',')) {
        wx_members.emplace_back(wstr);
    }
    return wx_members;
}

int add_chatroom_member(const string &roomid, const string &wxids)
{
    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return -1;
    }

    get_chatroom_mgr_t get_chatroom_mgr
        = reinterpret_cast<get_chatroom_mgr_t>(g_WeChatWinDllAddr + OS_GET_CHATROOM_MGR);
    add_member_to_chatroom_t add_members
        = reinterpret_cast<add_member_to_chatroom_t>(g_WeChatWinDllAddr + OS_ADD_MEMBERS);

    vector<WxString> wx_members = parse_wxids(wxids);
    auto wx_roomid              = util::new_wx_string(roomid);
    QWORD p_members             = reinterpret_cast<QWORD>(&wx_members.front());

    return static_cast<int>(add_members(get_chatroom_mgr(), p_members, reinterpret_cast<QWORD>(wx_roomid.get()), 0));
}

int del_chatroom_member(const string &roomid, const string &wxids)
{
    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return -1;
    }

    get_chatroom_mgr_t get_chatroom_mgr
        = reinterpret_cast<get_chatroom_mgr_t>(g_WeChatWinDllAddr + OS_GET_CHATROOM_MGR);
    del_member_from_chatroom_t del_members
        = reinterpret_cast<del_member_from_chatroom_t>(g_WeChatWinDllAddr + OS_DELETE_MEMBERS);

    vector<WxString> wx_members = parse_wxids(wxids);
    auto wx_roomid              = util::new_wx_string(roomid);
    QWORD p_members             = reinterpret_cast<QWORD>(&wx_members.front());

    return static_cast<int>(del_members(get_chatroom_mgr(), p_members, reinterpret_cast<QWORD>(wx_roomid.get())));
}

int invite_chatroom_member(const string &roomid, const string &wxids)
{
    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return -1;
    }

    invite_member_to_chatroom_t invite_members
        = reinterpret_cast<invite_member_to_chatroom_t>(g_WeChatWinDllAddr + OS_INVITE_MEMBERS);

    vector<WxString> wx_members = parse_wxids(wxids);
    auto wx_roomid              = util::new_wx_string(roomid);
    QWORD p_members             = reinterpret_cast<QWORD>(&wx_members.front());

    return static_cast<int>(invite_members(reinterpret_cast<QWORD>(wx_roomid.get()->wptr), p_members,
                                           reinterpret_cast<QWORD>(wx_roomid.get()), 0));
}

bool rpc_add_chatroom_member(const MemberMgmt &m, uint8_t *out, size_t *len)
{
    const std::string wxids  = m.wxids;
    const std::string roomid = m.roomid;
    return fill_response<Functions_FUNC_ADD_ROOM_MEMBERS>(
        out, len, [&](Response &rsp) { rsp.msg.status = add_chatroom_member(roomid, wxids); });
}

bool rpc_delete_chatroom_member(const MemberMgmt &m, uint8_t *out, size_t *len)
{
    const std::string wxids  = m.wxids;
    const std::string roomid = m.roomid;
    return fill_response<Functions_FUNC_DEL_ROOM_MEMBERS>(
        out, len, [&](Response &rsp) { rsp.msg.status = del_chatroom_member(roomid, wxids); });
}

bool rpc_invite_chatroom_member(const MemberMgmt &m, uint8_t *out, size_t *len)
{
    const std::string wxids  = m.wxids;
    const std::string roomid = m.roomid;
    return fill_response<Functions_FUNC_INV_ROOM_MEMBERS>(
        out, len, [&](Response &rsp) { rsp.msg.status = invite_chatroom_member(roomid, wxids); });
}

} // namespace chatroom
