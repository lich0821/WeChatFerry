#include "message_handler.h"

#include <condition_variable>
#include <mutex>
#include <queue>

#include "framework.h"

#include "account_manager.h"
#include "log.hpp"
#include "offsets.h"
#include "pb_util.h"
#include "rpc_helper.h"
#include "util.h"

extern QWORD g_WeChatWinDllAddr;

#define OS_RECV_MSG_ID      0x30
#define OS_RECV_MSG_TYPE    0x38
#define OS_RECV_MSG_SELF    0x3C
#define OS_RECV_MSG_TS      0x44
#define OS_RECV_MSG_ROOMID  0x48
#define OS_RECV_MSG_CONTENT 0x88
#define OS_RECV_MSG_WXID    0x240
#define OS_RECV_MSG_SIGN    0x260
#define OS_RECV_MSG_THUMB   0x280
#define OS_RECV_MSG_EXTRA   0x2A0
#define OS_RECV_MSG_XML     0x308
#define OS_RECV_MSG_CALL    0x213ED90
#define OS_PYQ_MSG_START    0x30
#define OS_PYQ_MSG_END      0x38
#define OS_PYQ_MSG_TS       0x38
#define OS_PYQ_MSG_XML      0x9B8
#define OS_PYQ_MSG_SENDER   0x18
#define OS_PYQ_MSG_CONTENT  0x48
#define OS_PYQ_MSG_CALL     0x2E42C90
#define OS_WXLOG            0x2613D20

namespace message
{
namespace OsLog = Offsets::Message::Log;

QWORD Handler::DispatchMsg(QWORD arg1, QWORD arg2)
{
    auto &handler = getInstance();
    WxMsg_t wxMsg = {};
    try {
        wxMsg.id      = util::get_qword(arg2 + OS_RECV_MSG_ID);
        wxMsg.type    = util::get_dword(arg2 + OS_RECV_MSG_TYPE);
        wxMsg.is_self = util::get_dword(arg2 + OS_RECV_MSG_SELF);
        wxMsg.ts      = util::get_dword(arg2 + OS_RECV_MSG_TS);
        wxMsg.content = util::get_str_by_wstr_addr(arg2 + OS_RECV_MSG_CONTENT);
        wxMsg.sign    = util::get_str_by_wstr_addr(arg2 + OS_RECV_MSG_SIGN);
        wxMsg.xml     = util::get_str_by_wstr_addr(arg2 + OS_RECV_MSG_XML);
        wxMsg.roomid  = util::get_str_by_wstr_addr(arg2 + OS_RECV_MSG_ROOMID);

        if (wxMsg.roomid.find("@chatroom") != std::string::npos) {
            wxMsg.is_group = true;
            wxMsg.sender
                = wxMsg.is_self ? account::get_self_wxid() : util::get_str_by_wstr_addr(arg2 + OS_RECV_MSG_WXID);
        } else {
            wxMsg.is_group = false;
            wxMsg.sender   = wxMsg.is_self ? account::get_self_wxid() : wxMsg.roomid;
        }
    } catch (const std::exception &e) {
        LOG_ERROR(util::gb2312_to_utf8(e.what()));
    }

    {
        std::unique_lock<std::mutex> lock(handler.mutex_);
        handler.msgQueue_.push(wxMsg);
    }

    handler.cv_.notify_all();
    return handler.realRecvMsg(arg1, arg2);
}

QWORD Handler::PrintWxLog(QWORD a1, QWORD a2, QWORD a3, QWORD a4, QWORD a5, QWORD a6, QWORD a7, QWORD a8, QWORD a9,
                          QWORD a10, QWORD a11, QWORD a12)
{
    auto &handler = getInstance();

    QWORD p = handler.realWxLog(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
    if (p == 0 || p == 1) {
        return p;
    }

    LOG_INFO("【WX】\n{}", util::gb2312_to_utf8((char *)p));
    return p;
}

void Handler::DispatchPyq(QWORD arg1, QWORD arg2, QWORD arg3)
{
    auto &handler   = getInstance();
    QWORD startAddr = *(QWORD *)(arg2 + OS_PYQ_MSG_START);
    QWORD endAddr   = *(QWORD *)(arg2 + OS_PYQ_MSG_END);

    if (startAddr == 0) {
        return;
    }

    while (startAddr < endAddr) {
        WxMsg_t wxMsg;

        wxMsg.type     = 0x00;
        wxMsg.is_self  = false;
        wxMsg.is_group = false;
        wxMsg.id       = util::get_qword(startAddr);
        wxMsg.ts       = util::get_dword(startAddr + OS_PYQ_MSG_TS);
        wxMsg.xml      = util::get_str_by_wstr_addr(startAddr + OS_PYQ_MSG_XML);
        wxMsg.sender   = util::get_str_by_wstr_addr(startAddr + OS_PYQ_MSG_SENDER);
        wxMsg.content  = util::get_str_by_wstr_addr(startAddr + OS_PYQ_MSG_CONTENT);

        {
            std::unique_lock<std::mutex> lock(handler.mutex_);
            handler.msgQueue_.push(wxMsg);
        }

        handler.cv_.notify_all();
        startAddr += 0x1618;
    }
}

Handler &Handler::getInstance()
{
    static Handler instance;
    return instance;
}

Handler::Handler()
{
    isLogging      = false;
    isListeningMsg = false;
    isListeningPyq = false;
}

Handler::~Handler()
{
    DisableLog();
    UnListenMsg();
    UnListenPyq();
}

MsgTypes_t Handler::GetMsgTypes()
{
    return { { 0x00, "朋友圈消息" },
             { 0x01, "文字" },
             { 0x03, "图片" },
             { 0x22, "语音" },
             { 0x25, "好友确认" },
             { 0x28, "POSSIBLEFRIEND_MSG" },
             { 0x2A, "名片" },
             { 0x2B, "视频" },
             { 0x2F, "石头剪刀布 | 表情图片" },
             { 0x30, "位置" },
             { 0x31, "共享实时位置、文件、转账、链接" },
             { 0x32, "VOIPMSG" },
             { 0x33, "微信初始化" },
             { 0x34, "VOIPNOTIFY" },
             { 0x35, "VOIPINVITE" },
             { 0x3E, "小视频" },
             { 0x42, "微信红包" },
             { 0x270F, "SYSNOTICE" },
             { 0x2710, "红包、系统消息" },
             { 0x2712, "撤回消息" },
             { 0x100031, "搜狗表情" },
             { 0x1000031, "链接" },
             { 0x1A000031, "微信红包" },
             { 0x20010031, "红包封面" },
             { 0x2D000031, "视频号视频" },
             { 0x2E000031, "视频号名片" },
             { 0x31000031, "引用消息" },
             { 0x37000031, "拍一拍" },
             { 0x3A000031, "视频号直播" },
             { 0x3A100031, "商品链接" },
             { 0x3A200031, "视频号直播" },
             { 0x3E000031, "音乐链接" },
             { 0x41000031, "文件" } };
}

std::optional<WxMsg_t> Handler::popMessage()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (msgQueue_.empty()) {
        return std::nullopt;
    }
    WxMsg_t msg = std::move(msgQueue_.front());
    msgQueue_.pop();
    return msg;
}

int Handler::EnableLog()
{
    if (isLogging) return 1;

    pLogLevel = reinterpret_cast<uint32_t *>(g_WeChatWinDllAddr + OsLog::LEVEL);
    funcWxLog = reinterpret_cast<funcWxLog_t>(g_WeChatWinDllAddr + OsLog::CALL);

    if (InitializeHook() != MH_OK) return -1;
    if (MH_CreateHook(funcWxLog, &PrintWxLog, reinterpret_cast<LPVOID *>(&realWxLog)) != MH_OK) return -2;
    if (MH_EnableHook(funcWxLog) != MH_OK) return -3;

    *pLogLevel = 0;
    isLogging  = true;
    return 0;
}

int Handler::DisableLog()
{
    if (!isLogging) return 1;
    if (MH_DisableHook(funcWxLog) != MH_OK) return -1;
    if (UninitializeHook() != MH_OK) return -2;
    *pLogLevel = 6;
    isLogging  = false;
    return 0;
}

int Handler::ListenMsg()
{
    if (isListeningMsg) return 1;

    funcRecvMsg = reinterpret_cast<funcRecvMsg_t>(g_WeChatWinDllAddr + OS_RECV_MSG_CALL);
    if (InitializeHook() != MH_OK) return -1;
    if (MH_CreateHook(funcRecvMsg, &DispatchMsg, reinterpret_cast<LPVOID *>(&realRecvMsg)) != MH_OK) return -1;
    if (MH_EnableHook(funcRecvMsg) != MH_OK) return -1;

    isListeningMsg = true;
    return 0;
}

int Handler::UnListenMsg()
{
    if (!isListeningMsg) return 1;
    if (MH_DisableHook(funcRecvMsg) != MH_OK) return -1;
    if (UninitializeHook() != MH_OK) return -1;
    isListeningMsg = false;
    return 0;
}

int Handler::ListenPyq()
{
    if (isListeningPyq) return 1;

    funcRecvPyq = reinterpret_cast<funcRecvPyq_t>(g_WeChatWinDllAddr + OS_PYQ_MSG_CALL);
    if (InitializeHook() != MH_OK) return -1;
    if (MH_CreateHook(funcRecvPyq, &DispatchPyq, reinterpret_cast<LPVOID *>(&realRecvPyq)) != MH_OK) return -1;
    if (MH_EnableHook(funcRecvPyq) != MH_OK) return -1;

    isListeningPyq = true;
    return 0;
}

int Handler::UnListenPyq()
{
    if (!isListeningPyq) return 1;
    if (MH_DisableHook(funcRecvPyq) != MH_OK) return -1;
    if (UninitializeHook() != MH_OK) return -1;
    isListeningPyq = false;
    return 0;
}

MH_STATUS Handler::InitializeHook()
{
    if (isMH_Initialized) return MH_OK;
    MH_STATUS status = MH_Initialize();
    if (status == MH_OK) isMH_Initialized = true;
    return status;
}

MH_STATUS Handler::UninitializeHook()
{
    if (!isMH_Initialized || isLogging || isListeningMsg || isListeningPyq) return MH_OK;
    MH_STATUS status = MH_Uninitialize();
    if (status == MH_OK) isMH_Initialized = false;
    return status;
}

bool Handler::rpc_get_msg_types(uint8_t *out, size_t *len)
{
    MsgTypes_t types                 = GetMsgTypes();
    return fill_response<Functions_FUNC_GET_MSG_TYPES>(out, len, [&](Response &rsp) {
        rsp.msg.types.types.funcs.encode = encode_types;
        rsp.msg.types.types.arg          = &types;
    });
}
}
