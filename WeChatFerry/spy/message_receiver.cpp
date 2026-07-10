#pragma execution_character_set("utf-8")

#include "message_receiver.h"

#include <algorithm>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <intrin.h>
#include <mutex>
#include <queue>

#include "account_manager.h"
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

using ReceiveMessageFn = uintptr_t(__thiscall *)(void *msg);
// flag 必须原样透传：OnSnsTimeLineSceneFinish 有 2 个调用者，丢参会破坏另一条（非接收）路径
using ReceivePyqFn     = uintptr_t(__thiscall *)(void *self, uint32_t data, uint32_t flag);

struct DetourHook {
    uint32_t target      = 0;
    void *trampoline     = nullptr;
    unsigned char saved[5] = { 0 };
    bool installed       = false;
};

ReceiveMessageFn gRealReceiveMessage = nullptr;
ReceivePyqFn gRealReceivePyq         = nullptr;
DetourHook gMessageHook;
DetourHook gPyqHook;

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

bool write_jump(uint32_t src, const void *dst)
{
    DWORD old_protect = 0;
    if (!VirtualProtect(reinterpret_cast<LPVOID>(src), 5, PAGE_EXECUTE_READWRITE, &old_protect)) {
        return false;
    }

    unsigned char jump[5] = { 0xE9, 0, 0, 0, 0 };
    *reinterpret_cast<uint32_t *>(&jump[1]) = reinterpret_cast<uint32_t>(dst) - src - 5;
    std::memcpy(reinterpret_cast<void *>(src), jump, sizeof(jump));
    FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<LPCVOID>(src), sizeof(jump));

    DWORD restored = 0;
    VirtualProtect(reinterpret_cast<LPVOID>(src), 5, old_protect, &restored);
    return true;
}

bool install_hook(DetourHook &hook, uint32_t target, const void *replacement, void **original)
{
    if (hook.installed) {
        *original = hook.trampoline;
        return true;
    }

    hook.target = target;
    std::memcpy(hook.saved, reinterpret_cast<void *>(target), sizeof(hook.saved));

    auto trampoline = static_cast<unsigned char *>(
        VirtualAlloc(nullptr, 16, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (trampoline == nullptr) {
        LOG_ERROR("VirtualAlloc failed for trampoline.");
        return false;
    }

    std::memcpy(trampoline, hook.saved, sizeof(hook.saved));
    trampoline[5] = 0xE9;
    *reinterpret_cast<uint32_t *>(trampoline + 6) = (target + 5) - (reinterpret_cast<uint32_t>(trampoline) + 10);

    if (!write_jump(target, replacement)) {
        VirtualFree(trampoline, 0, MEM_RELEASE);
        LOG_ERROR("Failed to patch hook target: 0x{:08X}", target);
        return false;
    }

    hook.trampoline = trampoline;
    hook.installed  = true;
    *original       = trampoline;
    return true;
}

void remove_hook(DetourHook &hook)
{
    if (!hook.installed) {
        return;
    }

    DWORD old_protect = 0;
    if (VirtualProtect(reinterpret_cast<LPVOID>(hook.target), 5, PAGE_EXECUTE_READWRITE, &old_protect)) {
        std::memcpy(reinterpret_cast<void *>(hook.target), hook.saved, sizeof(hook.saved));
        FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<LPCVOID>(hook.target), sizeof(hook.saved));
        DWORD restored = 0;
        VirtualProtect(reinterpret_cast<LPVOID>(hook.target), 5, old_protect, &restored);
    } else {
        LOG_ERROR("Failed to restore hook target: 0x{:08X}", hook.target);
    }

    if (hook.trampoline != nullptr) {
        VirtualFree(hook.trampoline, 0, MEM_RELEASE);
    }

    hook.target     = 0;
    hook.trampoline = nullptr;
    hook.installed  = false;
    std::memset(hook.saved, 0, sizeof(hook.saved));
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
        LOG_ERROR("Unknown exception.");
    }

    {
        std::unique_lock<std::mutex> lock(gMutex);
        gMsgQueue.push(wx_msg);
    }

    gCV.notify_all();
}

uintptr_t __fastcall receive_message_hook(void *msg, void *)
{
    // ~ChatMsg 有大量调用点，按“返回地址==HOOK+5”过滤，仅当本次由收消息的定点 call 触发时才 dispatch
    if (reinterpret_cast<uint32_t>(_ReturnAddress())
        == g_WeChatWinDllAddr + Offsets::Message::Receive::HOOK + 5) {
        dispatch_msg(reinterpret_cast<uint32_t>(msg));
    }
    return gRealReceiveMessage ? gRealReceiveMessage(msg) : 0;
}

void listen_message()
{
    if (gIsListening || (g_WeChatWinDllAddr == 0)) {
        return;
    }

    uint32_t target = g_WeChatWinDllAddr + Offsets::Message::Receive::CALL;
    if (install_hook(gMessageHook, target, reinterpret_cast<const void *>(receive_message_hook),
                     reinterpret_cast<void **>(&gRealReceiveMessage))) {
        gIsListening = true;
    }
}

void unlisten_message()
{
    if (!gIsListening) {
        return;
    }

    remove_hook(gMessageHook);
    gRealReceiveMessage = nullptr;
    gIsListening        = false;
}

void dispatch_pyq(uint32_t reg)
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

uintptr_t __fastcall receive_pyq_hook(void *self, void *, uint32_t data, uint32_t flag)
{
    // OnSnsTimeLineSceneFinish 有 2 个调用者，仅当由 OnProcessTimelineResp 定点触发（返回地址==HOOK+5）
    // 才是"刚收到一批朋友圈"，此时 data 即容器指针
    if (reinterpret_cast<uint32_t>(_ReturnAddress())
        == g_WeChatWinDllAddr + Offsets::Moments::HOOK + 5) {
        dispatch_pyq(data);
    }
    return gRealReceivePyq ? gRealReceivePyq(self, data, flag) : 0;
}

void listen_pyq()
{
    if (gIsListeningPyq || (g_WeChatWinDllAddr == 0)) {
        return;
    }

    uint32_t target = g_WeChatWinDllAddr + Offsets::Moments::CALL;
    if (install_hook(gPyqHook, target, reinterpret_cast<const void *>(receive_pyq_hook),
                     reinterpret_cast<void **>(&gRealReceivePyq))) {
        gIsListeningPyq = true;
    }
}

void unlisten_pyq()
{
    if (!gIsListeningPyq) {
        return;
    }

    remove_hook(gPyqHook);
    gRealReceivePyq = nullptr;
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
