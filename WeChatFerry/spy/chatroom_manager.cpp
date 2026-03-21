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

int add_chatroom_member(const std::string &roomid, const std::string &wxids)
{
    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return -1;
    }

    int rv            = 0;
    uint32_t armCall1 = g_WeChatWinDllAddr + Offsets::Chatroom::ADD_CALL1;
    uint32_t armCall2 = g_WeChatWinDllAddr + Offsets::Chatroom::ADD_CALL2;
    uint32_t armCall3 = g_WeChatWinDllAddr + Offsets::Chatroom::ADD_CALL3;

    uint32_t temp         = 0;
    std::wstring wsRoomid = util::s2w(roomid);
    WxString wxRoomid(wsRoomid);

    std::vector<std::wstring> vMembers;
    std::vector<WxString> vWxMembers;
    std::wstringstream wss(util::s2w(wxids));
    while (wss.good()) {
        std::wstring wstr;
        getline(wss, wstr, L',');
        vMembers.push_back(wstr);
        WxString txtMember(vMembers.back());
        vWxMembers.push_back(txtMember);
    }

    LOG_DEBUG("Adding {} members[{}] to {}", vWxMembers.size(), wxids.c_str(), roomid.c_str());
    __asm {
        pushad;
        pushfd;
        call armCall1;
        sub esp, 0x8;
        mov temp, eax;
        mov ecx, esp;
        mov dword ptr[ecx], 0x0;
        mov dword ptr[ecx + 4], 0x0;
        test esi, esi;
        sub esp, 0x14;
        mov ecx, esp;
        lea eax, wxRoomid;
        push eax;
        call armCall2;
        mov ecx, temp;
        lea eax, vWxMembers;
        push eax;
        call armCall3;
        mov rv, eax;
        popfd;
        popad;
    }
    return rv;
}

int del_chatroom_member(const std::string &roomid, const std::string &wxids)
{
    if (roomid.empty() || wxids.empty()) {
        LOG_ERROR("Empty roomid or wxids.");
        return -1;
    }

    int rv            = 0;
    uint32_t drmCall1 = g_WeChatWinDllAddr + Offsets::Chatroom::DEL_CALL1;
    uint32_t drmCall2 = g_WeChatWinDllAddr + Offsets::Chatroom::DEL_CALL2;
    uint32_t drmCall3 = g_WeChatWinDllAddr + Offsets::Chatroom::DEL_CALL3;

    uint32_t temp         = 0;
    std::wstring wsRoomid = util::s2w(roomid);
    WxString wxRoomid(wsRoomid);

    std::vector<std::wstring> vMembers;
    std::vector<WxString> vWxMembers;
    std::wstringstream wss(util::s2w(wxids));
    while (wss.good()) {
        std::wstring wstr;
        getline(wss, wstr, L',');
        vMembers.push_back(wstr);
        WxString txtMember(vMembers.back());
        vWxMembers.push_back(txtMember);
    }

    LOG_DEBUG("Deleting {} members[{}] from {}", vWxMembers.size(), wxids.c_str(), roomid.c_str());
    __asm {
        pushad;
        pushfd;
        call drmCall1;
        sub esp, 0x14;
        mov esi, eax;
        mov ecx, esp;
        lea edi, wxRoomid;
        push edi;
        call drmCall2;
        mov ecx, esi;
        lea eax, vWxMembers;
        push eax;
        call drmCall3;
        mov rv, eax;
        popfd;
        popad;
    }
    return rv;
}

int invite_chatroom_member(const std::string &roomid, const std::string &wxids)
{
    std::wstring wsRoomid = util::s2w(roomid);
    WxString wxRoomid(wsRoomid);

    std::vector<std::wstring> vMembers;
    std::vector<WxString> vWxMembers;
    std::wstringstream wss(util::s2w(wxids));
    while (wss.good()) {
        std::wstring wstr;
        getline(wss, wstr, L',');
        vMembers.push_back(wstr);
        WxString wxMember(vMembers.back());
        vWxMembers.push_back(wxMember);
    }

    LOG_DEBUG("Inviting {} members[{}] to {}", vWxMembers.size(), wxids.c_str(), roomid.c_str());

    uint32_t irmCall1 = g_WeChatWinDllAddr + Offsets::Chatroom::INV_CALL1;
    uint32_t irmCall2 = g_WeChatWinDllAddr + Offsets::Chatroom::INV_CALL2;
    uint32_t irmCall3 = g_WeChatWinDllAddr + Offsets::Chatroom::INV_CALL3;
    uint32_t irmCall4 = g_WeChatWinDllAddr + Offsets::Chatroom::INV_CALL4;
    uint32_t irmCall5 = g_WeChatWinDllAddr + Offsets::Chatroom::INV_CALL5;
    uint32_t irmCall6 = g_WeChatWinDllAddr + Offsets::Chatroom::INV_CALL6;
    uint32_t irmCall7 = g_WeChatWinDllAddr + Offsets::Chatroom::INV_CALL7;
    uint32_t irmCall8 = g_WeChatWinDllAddr + Offsets::Chatroom::INV_CALL8;

    uint32_t sys_addr = (DWORD)GetModuleHandleA("win32u.dll") + 0x116C;
    DWORD addr[2]     = { sys_addr, 0 };
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
