#pragma execution_character_set("utf-8")

#include "chatroom_manager.h"
#include "log.hpp"
#include "offsets.h"
#include "pb_util.h"
#include "rpc_helper.h"
#include "spy.h"
#include "util.h"

namespace chatroom
{
namespace OsRoom = Offsets::Chatroom;

using new_t            = QWORD (*)(QWORD, WxString *);
using get_mgr_t        = QWORD (*)();
using add_member_t     = QWORD (*)(QWORD, QWORD, WxString *, QWORD);
using delete_member_t  = QWORD (*)(QWORD, QWORD, WxString *);
using invite_members_t = QWORD (*)(const wchar_t *, QWORD, QWORD, QWORD);

template <auto FillFunc, typename Func>
bool rpc_chatroom_common(const MemberMgmt &m, uint8_t *out, size_t *len, Func func)
{
    int status = -1;
    if (m.wxids && m.roomid) {
        status = func(m.roomid, m.wxids);
    } else {
        LOG_ERROR("wxid 和 roomid 不能为空");
    }
    return fill_response<FillFunc>(out, len, [&](Response &rsp) { rsp.msg.status = status; });
}

int add_chatroom_member(const string &roomid, const string &wxids)
{
    auto get_chatroom_mgr = Spy::getFunction<get_mgr_t>(OsRoom::MGR);
    auto add_members      = Spy::getFunction<add_member_t>(OsRoom::ADD);

    WxString *wx_roomid = util::CreateWxString(roomid);

    QWORD tmp[2] = { 0 };

    auto split       = util::parse_wxids(wxids);
    auto &wx_members = split.wxWxids;
    QWORD p_members  = reinterpret_cast<QWORD>(&wx_members);

    return static_cast<int>(add_members(get_chatroom_mgr(), p_members, wx_roomid, reinterpret_cast<QWORD>(tmp)));
}

int del_chatroom_member(const string &roomid, const string &wxids)
{
    auto get_chatroom_mgr = Spy::getFunction<get_mgr_t>(OsRoom::MGR);
    auto del_members      = Spy::getFunction<delete_member_t>(OsRoom::DEL);

    WxString *wx_roomid = util::CreateWxString(roomid);

    auto split       = util::parse_wxids(wxids);
    auto &wx_members = split.wxWxids;
    QWORD p_members  = reinterpret_cast<QWORD>(&wx_members);

    return static_cast<int>(del_members(get_chatroom_mgr(), p_members, wx_roomid));
}

int invite_chatroom_member(const string &roomid, const string &wxids)
{
    auto init_roomid    = Spy::getFunction<new_t>(OsRoom::NEW);
    auto invite_members = Spy::getFunction<invite_members_t>(OsRoom::INV);

    wstring ws_roomid = util::s2w(roomid);
    WxString wx_roomid(ws_roomid);

    QWORD tmp[2]   = { 0 };
    QWORD array[4] = { 0 };

    auto split       = util::parse_wxids(wxids);
    auto &wx_members = split.wxWxids;
    QWORD p_members  = reinterpret_cast<QWORD>(&wx_members);
    QWORD p_roomid   = init_roomid(reinterpret_cast<QWORD>(&array), &wx_roomid);
    LOG_BUFFER((uint8_t *)*(QWORD *)(*(QWORD *)p_members), 40);

    return static_cast<int>(invite_members(ws_roomid.c_str(), p_members, p_roomid, reinterpret_cast<QWORD>(tmp)));
}

bool rpc_add_chatroom_member(const MemberMgmt &m, uint8_t *out, size_t *len)
{
    return rpc_chatroom_common<Functions_FUNC_ADD_ROOM_MEMBERS>(m, out, len, add_chatroom_member);
}

bool rpc_delete_chatroom_member(const MemberMgmt &m, uint8_t *out, size_t *len)
{
    return rpc_chatroom_common<Functions_FUNC_DEL_ROOM_MEMBERS>(m, out, len, del_chatroom_member);
}

bool rpc_invite_chatroom_member(const MemberMgmt &m, uint8_t *out, size_t *len)
{
    return rpc_chatroom_common<Functions_FUNC_INV_ROOM_MEMBERS>(m, out, len, invite_chatroom_member);
}

} // namespace chatroom
