#include "chatroom_manager.h"

#include <sstream>
#include <vector>

#include "framework.h"
#include "log.hpp"
#include "offsets.h"
#include "rpc_helper.h"
#include "spy.h"
#include "spy_types.h"
#include "util.h"

namespace chatroom
{

namespace
{

struct WxStringValue {
    const wchar_t *wptr;
    uint32_t size;
    uint32_t capacity;
    const char *ptr;
    uint32_t clen;
};

struct InviteAddrValue {
    uint32_t first;
    uint32_t second;
};

using ChatroomManagerGetterFn = void *(*)();
using AddMembersFn            = int(__thiscall *)(void *manager, const std::vector<WxString> *members,
                                       WxStringValue roomid, int64_t reserved);
using DelMembersFn            = int(__thiscall *)(void *manager, const std::vector<WxString> *members,
                                       WxStringValue roomid);
using SetupInviteManagerFn    = void(__thiscall *)(void *manager, DWORD *addr);
using WarmupInviteFn          = void (*)();
using BuildInviteAddrFn       = void(__thiscall *)(InviteAddrValue *out, DWORD *src);
using BuildInviteRoomFn       = void(__thiscall *)(WxStringValue *out, const WxString *src);
using InviteMembersFn         = int(__stdcall *)(const std::vector<WxString> *members, WxStringValue roomid,
                                                 InviteAddrValue addr_value);
using CommitInviteFn          = int(__thiscall *)(void *manager, int confirm, int reserved);
using CleanupInviteAddrFn     = void(__thiscall *)(DWORD *addr);

WxStringValue to_value(const WxString &value)
{
    return { value.wptr, value.size, value.capacity, value.ptr, value.clen };
}

} // namespace

int add_chatroom_member(const std::string &roomid, const std::string &wxids)
{
    (void)roomid;
    (void)wxids;
    LOG_ERROR("Not Implemented yet.");
    return -1;
}

int del_chatroom_member(const std::string &roomid, const std::string &wxids)
{
    (void)roomid;
    (void)wxids;
    LOG_ERROR("Not Implemented yet.");
    return -1;
}

int invite_chatroom_member(const std::string &roomid, const std::string &wxids)
{
    (void)roomid;
    (void)wxids;
    LOG_ERROR("Not Implemented yet.");
    return -1;
}

bool rpc_add_chatroom_member(const MemberMgmt &m, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_ADD_ROOM_MEMBERS>(out, len, [&m](Response &rsp) {
        if ((m.roomid == NULL) || (m.wxids == NULL)) {
            LOG_ERROR("Empty roomid or wxids.");
            rsp.msg.status = -1;
        } else {
            int status = add_chatroom_member(m.roomid, m.wxids);
            rsp.msg.status = status;
        }
    });
}

bool rpc_delete_chatroom_member(const MemberMgmt &m, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_DEL_ROOM_MEMBERS>(out, len, [&m](Response &rsp) {
        if ((m.roomid == NULL) || (m.wxids == NULL)) {
            LOG_ERROR("Empty roomid or wxids.");
            rsp.msg.status = -1;
        } else {
            int status = del_chatroom_member(m.roomid, m.wxids);
            rsp.msg.status = status;
        }
    });
}

bool rpc_invite_chatroom_member(const MemberMgmt &m, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_INV_ROOM_MEMBERS>(out, len, [&m](Response &rsp) {
        if ((m.roomid == NULL) || (m.wxids == NULL)) {
            LOG_ERROR("Empty roomid or wxids.");
            rsp.msg.status = -1;
        } else {
            int status = invite_chatroom_member(m.roomid, m.wxids);
            rsp.msg.status = status;
        }
    });
}

} // namespace chatroom
