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
// WxString::assign(dst,src,len)（__thiscall，mm_realloc 深拷贝）——与卡片/文件发送共用 RichText::ASSIGN
using WxStringAssignFn        = void *(__thiscall *)(void *dest, const wchar_t *src, int len);
// ChatRoomMgr::doAddMemberToChatRoom：纯 __thiscall(ecx=manager)，8 栈参（members 1 + roomid 5 + reserved 2），retn 0x20 被调清栈
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
    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return -1;
    }

    // 1) 共用 getter：取 ChatRoomMgr 单例（返回对象指针本体，直接用、不 deref）
    auto get_manager = reinterpret_cast<ChatroomManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Chatroom::MGR_GETTER);
    void *manager    = get_manager();
    if (manager == nullptr) {
        LOG_ERROR("Failed to get ChatRoomMgr.");
        return -1;
    }

    // 2) 成员列表：先建全部 std::wstring，再统一包成非拥有 WxString 视图，
    //    避免 vector 扩容使先前 WxString.wptr 失效。
    //    doAddMemberToChatRoom 只读取成员并深拷贝进 net scene，故非拥有视图即可。
    std::vector<std::wstring> vMembers;
    std::wstringstream wss(util::s2w(wxids));
    while (wss.good()) {
        std::wstring wstr;
        getline(wss, wstr, L',');
        if (!wstr.empty()) {
            vMembers.push_back(wstr);
        }
    }
    std::vector<WxString> vWxMembers;
    vWxMembers.reserve(vMembers.size());
    for (auto &m : vMembers) {
        vWxMembers.push_back(WxString(m));
    }

    // 3) roomid：doAddMemberToChatRoom 末尾会 mm_free roomid 的 wptr/ptr，
    //    故须用 ASSIGN 建 WeChat 拥有副本、按值传、绝不传 std::wstring 别名。
    auto assign           = reinterpret_cast<WxStringAssignFn>(g_WeChatWinDllAddr + Offsets::RichText::ASSIGN);
    std::wstring wsRoomid = util::s2w(roomid);
    WxString wxRoomid;
    assign(&wxRoomid, wsRoomid.c_str(), -1);

    LOG_DEBUG("Adding {} members[{}] to {}", vWxMembers.size(), wxids.c_str(), roomid.c_str());

    // 4) doAddMemberToChatRoom(manager, &members, roomidValue(按值), reserved=0)：
    //    纯 __thiscall(retn 0x20)，栈平衡、无需帧指针纠正。
    auto add_members = reinterpret_cast<AddMembersFn>(g_WeChatWinDllAddr + Offsets::Chatroom::ADD_MEMBER);
    return add_members(manager, &vWxMembers, to_value(wxRoomid), 0);
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
