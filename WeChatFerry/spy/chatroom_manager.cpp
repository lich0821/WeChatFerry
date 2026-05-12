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

// 邀请上下文 shared_ptr（2 dword）：{ref_count 控制块指针, 对象指针}；按值传 {0,0}=NULL 可选历史信息
struct InviteContext {
    uint32_t ctrl;
    uint32_t obj;
};

using ChatroomManagerGetterFn = void *(*)();
// WxString::assign(dst,src,len)（__thiscall，mm_realloc 深拷贝）——与卡片/文件发送共用 RichText::ASSIGN
using WxStringAssignFn        = void *(__thiscall *)(void *dest, const wchar_t *src, int len);
// ChatRoomMgr::doAddMemberToChatRoom：纯 __thiscall(ecx=manager)，8 栈参（members 1 + roomid 5 + reserved 2），retn 0x20 被调清栈
using AddMembersFn            = int(__thiscall *)(void *manager, const std::vector<WxString> *members,
                                       WxStringValue roomid, int64_t reserved);
using DelMembersFn            = int(__thiscall *)(void *manager, const std::vector<WxString> *members,
                                       WxStringValue roomid);
// NetSceneInviteChatRoomMember 构建器 + 内部 doScene 发送：__stdcall(members 指针, roomid 按值 5 dword, 上下文 shared_ptr 按值 2 dword)
using InviteMembersFn         = char(__stdcall *)(const std::vector<WxString> *members, WxStringValue roomid,
                                                  InviteContext context);

WxStringValue to_value(const WxString &value)
{
    return { value.wptr, value.size, value.capacity, value.ptr, value.clen };
}

// 把逗号分隔的 wxids 拆成非拥有 WxString 视图。storage 持有底层 std::wstring，必须与返回值同生命周期：
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

    // 1) 共用 getter：取 ChatRoomMgr 单例（返回对象指针本体，直接用、不 deref）
    auto get_manager = reinterpret_cast<ChatroomManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Chatroom::MGR_GETTER);
    void *manager    = get_manager();
    if (manager == nullptr) {
        LOG_ERROR("Failed to get ChatRoomMgr.");
        return -1;
    }

    // 2) 成员列表：非拥有 WxString 视图（doAddMemberToChatRoom 只读取并深拷贝进 net scene）。
    std::vector<std::wstring> vMembers;
    std::vector<WxString> vWxMembers = make_member_views(wxids, vMembers);

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
    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return -1;
    }

    // 1) 共用 getter：与加群同一个 ChatRoomMgr 单例（返回对象指针本体，直接用、不 deref）
    auto get_manager = reinterpret_cast<ChatroomManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Chatroom::MGR_GETTER);
    void *manager    = get_manager();
    if (manager == nullptr) {
        LOG_ERROR("Failed to get ChatRoomMgr.");
        return -1;
    }

    // 2) 成员列表：非拥有 WxString 视图（doDelMemberFromChatRoom 只读取并深拷贝进 net scene）。
    std::vector<std::wstring> vMembers;
    std::vector<WxString> vWxMembers = make_member_views(wxids, vMembers);

    // 3) roomid：doDelMemberFromChatRoom 末尾同样 mm_free roomid 的 wptr/ptr，须用 ASSIGN 建 WeChat 拥有副本、按值传。
    auto assign           = reinterpret_cast<WxStringAssignFn>(g_WeChatWinDllAddr + Offsets::RichText::ASSIGN);
    std::wstring wsRoomid = util::s2w(roomid);
    WxString wxRoomid;
    assign(&wxRoomid, wsRoomid.c_str(), -1);

    LOG_DEBUG("Deleting {} members[{}] from {}", vWxMembers.size(), wxids.c_str(), roomid.c_str());

    // 4) doDelMemberFromChatRoom(manager, &members, roomidValue(按值))：6 栈参 retn 0x18 被调清栈、栈平衡（比加群少 reserved）。
    auto del_members = reinterpret_cast<DelMembersFn>(g_WeChatWinDllAddr + Offsets::Chatroom::DEL_MEMBER);
    return del_members(manager, &vWxMembers, to_value(wxRoomid));
}

int invite_chatroom_member(const std::string &roomid, const std::string &wxids)
{
    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return -1;
    }

    // 1) 共用 getter：仅 warmup，确保 ChatRoomMgr 单例已构造（返回值丢弃）。
    //    邀请核心不经过 manager——NetScene 构建器是自足的 __stdcall，doScene 直接发送。
    auto get_manager = reinterpret_cast<ChatroomManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Chatroom::MGR_GETTER);
    if (get_manager() == nullptr) {
        LOG_ERROR("Failed to get ChatRoomMgr.");
        return -1;
    }

    // 2) 成员列表：非拥有 WxString 视图（构建器只读取并深拷贝进 net scene 请求）。
    std::vector<std::wstring> vMembers;
    std::vector<WxString> vWxMembers = make_member_views(wxids, vMembers);

    // 3) roomid：构建器末尾会 mm_free roomid 的 wptr/ptr，故须用 ASSIGN 建 WeChat 拥有副本、按值传。
    auto assign           = reinterpret_cast<WxStringAssignFn>(g_WeChatWinDllAddr + Offsets::RichText::ASSIGN);
    std::wstring wsRoomid = util::s2w(roomid);
    WxString wxRoomid;
    assign(&wxRoomid, wsRoomid.c_str(), -1);

    LOG_DEBUG("Inviting {} members[{}] to {}", vWxMembers.size(), wxids.c_str(), roomid.c_str());

    // 4) 构建器(members, roomidValue 按值, 上下文 shared_ptr={0,0} NULL)：
    //    内部按 roomid 是否 "@im.chatroom" 结尾分流普通/OpenIM，并 SceneCenter::doScene 发送。
    //    __stdcall 被调清栈，无需帧指针纠正。返回 1 表示已入队。
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
