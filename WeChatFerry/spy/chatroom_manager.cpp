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

// 邀请上下文 shared_ptr（2 dword）：{控制块指针, 对象指针}；按值传 {0,0}=NULL
struct InviteContext {
    uint32_t ctrl;
    uint32_t obj;
};

using ChatroomManagerGetterFn = void *(*)();
using WxStringAssignFn        = void *(__thiscall *)(void *dest, const wchar_t *src, int len);
using AddMembersFn            = int(__thiscall *)(void *manager, const std::vector<WxString> *members,
                                       WxStringValue roomid, int64_t reserved);
using DelMembersFn            = int(__thiscall *)(void *manager, const std::vector<WxString> *members,
                                       WxStringValue roomid);
using InviteMembersFn         = char(__stdcall *)(const std::vector<WxString> *members, WxStringValue roomid,
                                                  InviteContext context);

WxStringValue to_value(const WxString &value)
{
    return { value.wptr, value.size, value.capacity, value.ptr, value.clen };
}

// 把逗号分隔的 wxids 拆成非拥有 WxString 视图；storage 持有底层 std::wstring，须与返回值同生命周期。
// 先建全部 std::wstring 再统一包视图，避免 vector 扩容使先前 WxString.wptr 失效。
std::vector<WxString> make_member_views(const std::string &wxids, std::vector<std::wstring> &storage)
{
    std::wstringstream wss(util::s2w(wxids));
    while (wss.good()) {
        std::wstring wstr;
        getline(wss, wstr, L',');
        if (!wstr.empty()) {
            storage.push_back(wstr);
        }
    }
    std::vector<WxString> views;
    views.reserve(storage.size());
    for (auto &m : storage) {
        views.push_back(WxString(m));
    }
    return views;
}

} // namespace

int add_chatroom_member(const std::string &roomid, const std::string &wxids)
{
    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return -1;
    }

    auto get_manager = reinterpret_cast<ChatroomManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Chatroom::MGR_GETTER);
    void *manager    = get_manager();
    if (manager == nullptr) {
        LOG_ERROR("Failed to get ChatRoomMgr.");
        return -1;
    }

    // 成员列表用非拥有视图即可（doAddMemberToChatRoom 只读取并深拷贝进 net scene）
    std::vector<std::wstring> vMembers;
    std::vector<WxString> vWxMembers = make_member_views(wxids, vMembers);

    // roomid 会被 doAddMemberToChatRoom mm_free，须用 ASSIGN 建 WeChat 拥有副本
    auto assign           = reinterpret_cast<WxStringAssignFn>(g_WeChatWinDllAddr + Offsets::RichText::ASSIGN);
    std::wstring wsRoomid = util::s2w(roomid);
    WxString wxRoomid;
    assign(&wxRoomid, wsRoomid.c_str(), -1);

    LOG_DEBUG("Adding {} members[{}] to {}", vWxMembers.size(), wxids.c_str(), roomid.c_str());

    auto add_members = reinterpret_cast<AddMembersFn>(g_WeChatWinDllAddr + Offsets::Chatroom::ADD_MEMBER);
    return add_members(manager, &vWxMembers, to_value(wxRoomid), 0);
}

int del_chatroom_member(const std::string &roomid, const std::string &wxids)
{
    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return -1;
    }

    auto get_manager = reinterpret_cast<ChatroomManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Chatroom::MGR_GETTER);
    void *manager    = get_manager();
    if (manager == nullptr) {
        LOG_ERROR("Failed to get ChatRoomMgr.");
        return -1;
    }

    std::vector<std::wstring> vMembers;
    std::vector<WxString> vWxMembers = make_member_views(wxids, vMembers);

    // roomid 会被 doDelMemberFromChatRoom mm_free，须用 ASSIGN 建 WeChat 拥有副本
    auto assign           = reinterpret_cast<WxStringAssignFn>(g_WeChatWinDllAddr + Offsets::RichText::ASSIGN);
    std::wstring wsRoomid = util::s2w(roomid);
    WxString wxRoomid;
    assign(&wxRoomid, wsRoomid.c_str(), -1);

    LOG_DEBUG("Deleting {} members[{}] from {}", vWxMembers.size(), wxids.c_str(), roomid.c_str());

    auto del_members = reinterpret_cast<DelMembersFn>(g_WeChatWinDllAddr + Offsets::Chatroom::DEL_MEMBER);
    return del_members(manager, &vWxMembers, to_value(wxRoomid));
}

int invite_chatroom_member(const std::string &roomid, const std::string &wxids)
{
    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return -1;
    }

    // 邀请核心不经过 manager，此处仅 warmup 确保 ChatRoomMgr 单例已构造
    auto get_manager = reinterpret_cast<ChatroomManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Chatroom::MGR_GETTER);
    if (get_manager() == nullptr) {
        LOG_ERROR("Failed to get ChatRoomMgr.");
        return -1;
    }

    std::vector<std::wstring> vMembers;
    std::vector<WxString> vWxMembers = make_member_views(wxids, vMembers);

    // roomid 会被构建器 mm_free，须用 ASSIGN 建 WeChat 拥有副本
    auto assign           = reinterpret_cast<WxStringAssignFn>(g_WeChatWinDllAddr + Offsets::RichText::ASSIGN);
    std::wstring wsRoomid = util::s2w(roomid);
    WxString wxRoomid;
    assign(&wxRoomid, wsRoomid.c_str(), -1);

    LOG_DEBUG("Inviting {} members[{}] to {}", vWxMembers.size(), wxids.c_str(), roomid.c_str());

    // 构建器内部按 roomid 是否以 "@im.chatroom" 结尾分流普通/OpenIM，并 doScene 发送
    auto invite_members = reinterpret_cast<InviteMembersFn>(g_WeChatWinDllAddr + Offsets::Chatroom::INVITE_MEMBER);
    return invite_members(&vWxMembers, to_value(wxRoomid), InviteContext{ 0, 0 });
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
