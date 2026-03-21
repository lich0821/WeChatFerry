#pragma execution_character_set("utf-8")

#include "message_receiver.h"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <queue>

#include "account_manager.h"
#include "framework.h"
#include "log.hpp"
#include "offsets.h"
#include "pb_util.h"
#include "rpc_helper.h"
#include "spy.h"
#include "util.h"

extern bool gIsListening, gIsListeningPyq;
extern std::mutex gMutex;
extern std::condition_variable gCV;
extern std::queue<WxMsg_t> gMsgQueue;

namespace
{

uint32_t recv_msg_hook_addr      = 0;
uint32_t recv_msg_call_addr      = 0;
uint32_t recv_msg_jump_back_addr = 0;
char recv_msg_backup_code[5]     = { 0 };

uint32_t recv_pyq_hook_addr      = 0;
uint32_t recv_pyq_call_addr      = 0;
uint32_t recv_pyq_jump_back_addr = 0;
char recv_pyq_backup_code[5]     = { 0 };

MsgTypes_t build_msg_types()
{
    return { { 0x00, "moments" },
             { 0x01, "text" },
             { 0x03, "image" },
             { 0x22, "voice" },
             { 0x25, "friend_confirm" },
             { 0x28, "possible_friend" },
             { 0x2A, "card" },
             { 0x2B, "video" },
             { 0x2F, "emoji_or_game" },
             { 0x30, "location" },
             { 0x31, "app_msg" },
             { 0x32, "voip_msg" },
             { 0x33, "init" },
             { 0x34, "voip_notify" },
             { 0x35, "voip_invite" },
             { 0x3E, "short_video" },
             { 0x42, "red_packet" },
             { 0x270F, "sys_notice" },
             { 0x2710, "system_or_red_packet" },
             { 0x2712, "recalled" },
             { 0x100031, "sogou_emoji" },
             { 0x1000031, "link" },
             { 0x1A000031, "wechat_red_packet" },
             { 0x20010031, "packet_cover" },
             { 0x2D000031, "channels_video" },
             { 0x2E000031, "channels_card" },
             { 0x31000031, "quote" },
             { 0x37000031, "pat" },
             { 0x3A000031, "channels_live" },
             { 0x3A100031, "product_link" },
             { 0x3A200031, "channels_live_2" },
             { 0x3E000031, "music_link" },
             { 0x41000031, "file" } };
}

void hook_address(uint32_t hook_addr, LPVOID func_addr, char backup_code[5])
{
    BYTE jmp_code[5] = { 0 };
    jmp_code[0]      = 0xE9;
    *reinterpret_cast<uint32_t *>(&jmp_code[1]) = reinterpret_cast<uint32_t>(func_addr) - hook_addr - 5;

    ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(hook_addr), backup_code, 5, 0);
    WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(hook_addr), jmp_code, 5, 0);
}

void unhook_address(uint32_t hook_addr, char restore_code[5])
{
    WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(hook_addr), restore_code, 5, 0);
}

void dispatch_msg(uint32_t reg)
{
    WxMsg_t wx_msg = {};
    try {
        wx_msg.id      = util::get_qword(reg + Offsets::Message::Receive::MSG_ID);
        wx_msg.type    = util::get_dword(reg + Offsets::Message::Receive::TYPE);
        wx_msg.is_self = util::get_dword(reg + Offsets::Message::Receive::IS_SELF);
        wx_msg.ts      = util::get_dword(reg + Offsets::Message::Receive::TS);
        wx_msg.content = util::get_string_by_wstr_addr(reg + Offsets::Message::Receive::CONTENT);
        wx_msg.sign    = util::get_string_by_str_addr(reg + Offsets::Message::Receive::SIGN);
        wx_msg.xml     = util::get_string_by_str_addr(reg + Offsets::Message::Receive::MSG_XML);

        std::string roomid = util::get_string_by_wstr_addr(reg + Offsets::Message::Receive::ROOM_ID);
        if (roomid.find("@chatroom") != std::string::npos) {
            wx_msg.is_group = true;
            wx_msg.roomid   = roomid;
            wx_msg.sender   = wx_msg.is_self ? account::get_self_wxid()
                                             : util::get_string_by_str_addr(reg + Offsets::Message::Receive::WXID);
        } else {
            wx_msg.is_group = false;
            wx_msg.sender   = wx_msg.is_self ? account::get_self_wxid() : roomid;
        }

        wx_msg.thumb = util::get_string_by_str_addr(reg + Offsets::Message::Receive::THUMB);
        if (!wx_msg.thumb.empty()) {
            wx_msg.thumb = account::get_home_path() + wx_msg.thumb;
            std::replace(wx_msg.thumb.begin(), wx_msg.thumb.end(), '\\', '/');
        }

        wx_msg.extra = util::get_string_by_str_addr(reg + Offsets::Message::Receive::EXTRA);
        if (!wx_msg.extra.empty()) {
            wx_msg.extra = account::get_home_path() + wx_msg.extra;
            std::replace(wx_msg.extra.begin(), wx_msg.extra.end(), '\\', '/');
        }
    } catch (const std::exception &e) {
        LOG_ERROR(util::gb2312_to_utf8(e.what()));
    } catch (...) {
        LOG_ERROR("Unknow exception.");
    }

    {
        std::unique_lock<std::mutex> lock(gMutex);
        gMsgQueue.push(wx_msg);
    }

    gCV.notify_all();
}

static __declspec(naked) void receive_msg_func()
{
    __asm {
        pushad
        pushfd
        push ecx
        call dispatch_msg
        add esp, 0x4
        popfd
        popad
        call recv_msg_call_addr
        jmp recv_msg_jump_back_addr
    }
}

void listen_message()
{
    if (gIsListening || (g_WeChatWinDllAddr == 0)) {
        return;
    }

    recv_msg_hook_addr      = g_WeChatWinDllAddr + Offsets::Message::Receive::HOOK;
    recv_msg_call_addr      = g_WeChatWinDllAddr + Offsets::Message::Receive::CALL;
    recv_msg_jump_back_addr = recv_msg_hook_addr + 5;

    hook_address(recv_msg_hook_addr, reinterpret_cast<LPVOID>(receive_msg_func), recv_msg_backup_code);
    gIsListening = true;
}

void unlisten_message()
{
    if (!gIsListening) {
        return;
    }

    unhook_address(recv_msg_hook_addr, recv_msg_backup_code);
    gIsListening = false;
}

void dispatch_pyq(DWORD reg)
{
    uint32_t start_addr = *reinterpret_cast<DWORD *>(reg + Offsets::Moments::START);
    uint32_t end_addr   = *reinterpret_cast<DWORD *>(reg + Offsets::Moments::END);

    if (start_addr == 0) {
        return;
    }

    while (start_addr < end_addr) {
        WxMsg_t wx_msg = {};

        wx_msg.type     = 0x00;
        wx_msg.is_self  = false;
        wx_msg.is_group = false;
        wx_msg.id       = util::get_qword(start_addr);
        wx_msg.ts       = util::get_dword(start_addr + Offsets::Moments::TS);
        wx_msg.xml      = util::get_string_by_wstr_addr(start_addr + Offsets::Moments::XML);
        wx_msg.sender   = util::get_string_by_wstr_addr(start_addr + Offsets::Moments::WXID);
        wx_msg.content  = util::get_string_by_wstr_addr(start_addr + Offsets::Moments::CONTENT);

        {
            std::unique_lock<std::mutex> lock(gMutex);
            gMsgQueue.push(wx_msg);
        }

        gCV.notify_all();
        start_addr += Offsets::Moments::STEP;
    }
}

static __declspec(naked) void receive_pyq_func()
{
    __asm {
        pushad
        pushfd
        push [esp + 0x24]
        call dispatch_pyq
        add esp, 0x4
        popfd
        popad
        call recv_pyq_call_addr
        jmp recv_pyq_jump_back_addr
    }
}

void listen_pyq()
{
    if (gIsListeningPyq || (g_WeChatWinDllAddr == 0)) {
        return;
    }

    recv_pyq_hook_addr      = g_WeChatWinDllAddr + Offsets::Moments::HOOK;
    recv_pyq_call_addr      = g_WeChatWinDllAddr + Offsets::Moments::CALL;
    recv_pyq_jump_back_addr = recv_pyq_hook_addr + 5;

    hook_address(recv_pyq_hook_addr, reinterpret_cast<LPVOID>(receive_pyq_func), recv_pyq_backup_code);
    gIsListeningPyq = true;
}

void unlisten_pyq()
{
    if (!gIsListeningPyq) {
        return;
    }

    unhook_address(recv_pyq_hook_addr, recv_pyq_backup_code);
    gIsListeningPyq = false;
}

} // namespace

namespace message
{

MsgTypes_t get_msg_types()
{
    return build_msg_types();
}

bool rpc_get_msg_types(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_MSG_TYPES>(out, len, [](Response &rsp) {
        MsgTypes_t types                 = get_msg_types();
        rsp.msg.types.types.funcs.encode = encode_types;
        rsp.msg.types.types.arg          = &types;
    });
}

bool rpc_enable_recv_txt(bool pyq, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_ENABLE_RECV_TXT>(out, len, [pyq](Response &rsp) {
        if (pyq) {
            listen_pyq();
        } else {
            listen_message();
        }
        rsp.msg.status = 0;
    });
}

bool rpc_disable_recv_txt(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_DISABLE_RECV_TXT>(out, len, [](Response &rsp) {
        stop_receiving();
        rsp.msg.status = 0;
    });
}

void stop_receiving()
{
    unlisten_message();
    unlisten_pyq();
}

} // namespace message
