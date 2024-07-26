#pragma execution_character_set("utf-8")

#include "MinHook.h"
#include "framework.h"
#include <condition_variable>
#include <mutex>
#include <queue>

#include "log.h"
#include "receive_msg.h"
#include "user_info.h"
#include "util.h"

// Defined in rpc_server.cpp
extern bool gIsLogging, gIsListening, gIsListeningPyq;
extern mutex gMutex;
extern condition_variable gCV;
extern queue<WxMsg_t> gMsgQueue;

// Defined in spy.cpp
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
#define OS_RECV_MSG_CALL    0x2205510
#define OS_PYQ_MSG_START    0x30
#define OS_PYQ_MSG_END      0x38
#define OS_PYQ_MSG_TS       0x38
#define OS_PYQ_MSG_XML      0x9B8
#define OS_PYQ_MSG_SENDER   0x18
#define OS_PYQ_MSG_CONTENT  0x48
#define OS_PYQ_MSG_CALL     0x2EFAA10
#define OS_WXLOG            0x26DA2D0

typedef QWORD (*RecvMsg_t)(QWORD, QWORD);
typedef QWORD (*WxLog_t)(QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD);
typedef QWORD (*RecvPyq_t)(QWORD, QWORD, QWORD);

static RecvMsg_t funcRecvMsg = nullptr;
static RecvMsg_t realRecvMsg = nullptr;
static WxLog_t funcWxLog     = nullptr;
static WxLog_t realWxLog     = nullptr;
static RecvPyq_t funcRecvPyq = nullptr;
static RecvPyq_t realRecvPyq = nullptr;
static bool isMH_Initialized = false;

MsgTypes_t GetMsgTypes()
{
    const MsgTypes_t m = {
        { 0x00, "朋友圈消息" },
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
        { 0x41000031, "文件" },
    };

    return m;
}

static QWORD DispatchMsg(QWORD arg1, QWORD arg2)
{
    WxMsg_t wxMsg = { 0 };
    try {
        wxMsg.id      = GET_QWORD(arg2 + OS_RECV_MSG_ID);
        wxMsg.type    = GET_DWORD(arg2 + OS_RECV_MSG_TYPE);
        wxMsg.is_self = GET_DWORD(arg2 + OS_RECV_MSG_SELF);
        wxMsg.ts      = GET_DWORD(arg2 + OS_RECV_MSG_TS);
        wxMsg.content = GetStringByWstrAddr(arg2 + OS_RECV_MSG_CONTENT);
        wxMsg.sign    = GetStringByWstrAddr(arg2 + OS_RECV_MSG_SIGN);
        wxMsg.xml     = GetStringByWstrAddr(arg2 + OS_RECV_MSG_XML);

        string roomid = GetStringByWstrAddr(arg2 + OS_RECV_MSG_ROOMID);
        wxMsg.roomid  = roomid;
        if (roomid.find("@chatroom") != string::npos) { // 群 ID 的格式为 xxxxxxxxxxx@chatroom
            wxMsg.is_group = true;
            if (wxMsg.is_self) {
                wxMsg.sender = GetSelfWxid();
            } else {
                wxMsg.sender = GetStringByWstrAddr(arg2 + OS_RECV_MSG_WXID);
            }
        } else {
            wxMsg.is_group = false;
            if (wxMsg.is_self) {
                wxMsg.sender = GetSelfWxid();
            } else {
                wxMsg.sender = roomid;
            }
        }

        wxMsg.thumb = GetStringByWstrAddr(arg2 + OS_RECV_MSG_THUMB);
        if (!wxMsg.thumb.empty()) {
            wxMsg.thumb = GetHomePath() + wxMsg.thumb;
            replace(wxMsg.thumb.begin(), wxMsg.thumb.end(), '\\', '/');
        }

        wxMsg.extra = GetStringByWstrAddr(arg2 + OS_RECV_MSG_EXTRA);
        if (!wxMsg.extra.empty()) {
            wxMsg.extra = GetHomePath() + wxMsg.extra;
            replace(wxMsg.extra.begin(), wxMsg.extra.end(), '\\', '/');
        }
    } catch (const std::exception &e) {
        LOG_ERROR(GB2312ToUtf8(e.what()));
    } catch (...) {
        LOG_ERROR("Unknow exception.");
    }

    {
        unique_lock<mutex> lock(gMutex);
        gMsgQueue.push(wxMsg); // 推送到队列
    }

    gCV.notify_all(); // 通知各方消息就绪
    return realRecvMsg(arg1, arg2);
}

static QWORD PrintWxLog(QWORD a1, QWORD a2, QWORD a3, QWORD a4, QWORD a5, QWORD a6, QWORD a7, QWORD a8, QWORD a9,
                        QWORD a10, QWORD a11, QWORD a12)
{
    QWORD p = realWxLog(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
    if (p == 0 || p == 1) {
        return p;
    }

    LOG_INFO("【WX】\n{}", GB2312ToUtf8((char *)p));

    return p;
}

static void DispatchPyq(QWORD arg1, QWORD arg2, QWORD arg3)
{
    QWORD startAddr = *(QWORD *)(arg2 + OS_PYQ_MSG_START);
    QWORD endAddr   = *(QWORD *)(arg2 + OS_PYQ_MSG_END);

    if (startAddr == 0) {
        return;
    }

    while (startAddr < endAddr) {
        WxMsg_t wxMsg;

        wxMsg.type     = 0x00; // 朋友圈消息
        wxMsg.is_self  = false;
        wxMsg.is_group = false;
        wxMsg.id       = GET_QWORD(startAddr);
        wxMsg.ts       = GET_DWORD(startAddr + OS_PYQ_MSG_TS);
        wxMsg.xml      = GetStringByWstrAddr(startAddr + OS_PYQ_MSG_XML);
        wxMsg.sender   = GetStringByWstrAddr(startAddr + OS_PYQ_MSG_SENDER);
        wxMsg.content  = GetStringByWstrAddr(startAddr + OS_PYQ_MSG_CONTENT);

        {
            unique_lock<mutex> lock(gMutex);
            gMsgQueue.push(wxMsg); // 推送到队列
        }

        gCV.notify_all(); // 通知各方消息就绪

        startAddr += 0x1618;
    }
}

static MH_STATUS InitializeHook()
{
    if (isMH_Initialized) {
        return MH_OK;
    }
    MH_STATUS status = MH_Initialize();
    if (status == MH_OK) {
        isMH_Initialized = true;
    }
    return status;
}

static MH_STATUS UninitializeHook()
{
    if (!isMH_Initialized) {
        return MH_OK;
    }
    if (gIsLogging || gIsListening || gIsListeningPyq) {
        return MH_OK;
    }
    MH_STATUS status = MH_Uninitialize();
    if (status == MH_OK) {
        isMH_Initialized = false;
    }
    return status;
}

void EnableLog()
{
    MH_STATUS status = MH_UNKNOWN;
    if (gIsLogging) {
        LOG_WARN("gIsLogging");
        return;
    }
    WxLog_t funcWxLog = (WxLog_t)(g_WeChatWinDllAddr + OS_WXLOG);

    status = InitializeHook();
    if (status != MH_OK) {
        LOG_ERROR("MH_Initialize failed: {}", to_string(status));
        return;
    }

    status = MH_CreateHook(funcWxLog, &PrintWxLog, reinterpret_cast<LPVOID *>(&realWxLog));
    if (status != MH_OK) {
        LOG_ERROR("MH_CreateHook failed: {}", to_string(status));
        return;
    }

    status = MH_EnableHook(funcWxLog);
    if (status != MH_OK) {
        LOG_ERROR("MH_EnableHook failed: {}", to_string(status));
        return;
    }
    gIsLogging = true;
}

void DisableLog()
{
    MH_STATUS status = MH_UNKNOWN;
    if (!gIsLogging) {
        return;
    }

    status = MH_DisableHook(funcWxLog);
    if (status != MH_OK) {
        LOG_ERROR("MH_DisableHook failed: {}", to_string(status));
        return;
    }

    gIsLogging = false;

    status = UninitializeHook();
    if (status != MH_OK) {
        LOG_ERROR("MH_Uninitialize failed: {}", to_string(status));
        return;
    }
}

void ListenMessage()
{
    MH_STATUS status = MH_UNKNOWN;
    if (gIsListening) {
        LOG_WARN("gIsListening");
        return;
    }
    funcRecvMsg = (RecvMsg_t)(g_WeChatWinDllAddr + OS_RECV_MSG_CALL);

    status = InitializeHook();
    if (status != MH_OK) {
        LOG_ERROR("MH_Initialize failed: {}", to_string(status));
        return;
    }

    status = MH_CreateHook(funcRecvMsg, &DispatchMsg, reinterpret_cast<LPVOID *>(&realRecvMsg));
    if (status != MH_OK) {
        LOG_ERROR("MH_CreateHook failed: {}", to_string(status));
        return;
    }

    status = MH_EnableHook(funcRecvMsg);
    if (status != MH_OK) {
        LOG_ERROR("MH_EnableHook failed: {}", to_string(status));
        return;
    }

    gIsListening = true;
}

void UnListenMessage()
{
    MH_STATUS status = MH_UNKNOWN;
    if (!gIsListening) {
        return;
    }

    status = MH_DisableHook(funcRecvMsg);
    if (status != MH_OK) {
        LOG_ERROR("MH_DisableHook failed: {}", to_string(status));
        return;
    }

    gIsListening = false;

    status = UninitializeHook();
    if (status != MH_OK) {
        LOG_ERROR("MH_Uninitialize failed: {}", to_string(status));
        return;
    }
}

void ListenPyq()
{
    MH_STATUS status = MH_UNKNOWN;
    if (gIsListeningPyq) {
        LOG_WARN("gIsListeningPyq");
        return;
    }
    funcRecvPyq = (RecvPyq_t)(g_WeChatWinDllAddr + OS_PYQ_MSG_CALL);

    status = InitializeHook();
    if (status != MH_OK) {
        LOG_ERROR("MH_Initialize failed: {}", to_string(status));
        return;
    }

    status = MH_CreateHook(funcRecvPyq, &DispatchPyq, reinterpret_cast<LPVOID *>(&realRecvPyq));
    if (status != MH_OK) {
        LOG_ERROR("MH_CreateHook failed: {}", to_string(status));
        return;
    }

    status = MH_EnableHook(funcRecvPyq);
    if (status != MH_OK) {
        LOG_ERROR("MH_EnableHook failed: {}", to_string(status));
        return;
    }

    gIsListeningPyq = true;
}

void UnListenPyq()
{
    MH_STATUS status = MH_UNKNOWN;
    if (!gIsListeningPyq) {
        return;
    }

    status = MH_DisableHook(funcRecvPyq);
    if (status != MH_OK) {
        LOG_ERROR("MH_DisableHook failed: {}", to_string(status));
        return;
    }

    gIsListeningPyq = false;

    status = UninitializeHook();
    if (status != MH_OK) {
        LOG_ERROR("MH_Uninitialize failed: {}", to_string(status));
        return;
    }
}
