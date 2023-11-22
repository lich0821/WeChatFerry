#pragma execution_character_set("utf-8")

#include "framework.h"
#include <condition_variable>
#include <mutex>
#include <queue>

#include "load_calls.h"
#include "log.h"
#include "receive_msg.h"
#include "user_info.h"
#include "util.h"

// Defined in rpc_server.cpp
extern bool gIsListening, gIsListeningPyq;
extern mutex gMutex;
extern condition_variable gCV;
extern queue<WxMsg_t> gMsgQueue;

// Defined in spy.cpp
extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

static DWORD reg_buffer          = 0;
static DWORD recvMsgHookAddr     = 0;
static DWORD recvMsgCallAddr     = 0;
static DWORD recvMsgJumpBackAddr = 0;
static CHAR recvMsgBackupCode[5] = { 0 };

static DWORD recvPyqHookAddr     = 0;
static DWORD recvPyqCallAddr     = 0;
static DWORD recvPyqJumpBackAddr = 0;
static CHAR recvPyqBackupCode[5] = { 0 };

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

void HookAddress(DWORD hookAddr, LPVOID funcAddr, CHAR recvMsgBackupCode[5])
{
    // 组装跳转数据
    BYTE jmpCode[5] = { 0 };
    jmpCode[0]      = 0xE9;

    // 计算偏移
    *(DWORD *)&jmpCode[1] = (DWORD)funcAddr - hookAddr - 5;

    // 备份原来的代码
    ReadProcessMemory(GetCurrentProcess(), (LPVOID)hookAddr, recvMsgBackupCode, 5, 0);
    // 写入新的代码
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddr, jmpCode, 5, 0);
}

void UnHookAddress(DWORD hookAddr, CHAR restoreCode[5])
{
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddr, restoreCode, 5, 0);
}

void DispatchMsg(DWORD reg)
{
    WxMsg_t wxMsg;
    try {
        wxMsg.id      = GET_QWORD(reg + g_WxCalls.recvMsg.msgId);
        wxMsg.type    = GET_DWORD(reg + g_WxCalls.recvMsg.type);
        wxMsg.is_self = GET_DWORD(reg + g_WxCalls.recvMsg.isSelf);
        wxMsg.ts      = GET_DWORD(reg + g_WxCalls.recvMsg.ts);
        wxMsg.content = GetStringByWstrAddr(reg + g_WxCalls.recvMsg.content);
        wxMsg.sign    = GetStringByStrAddr(reg + g_WxCalls.recvMsg.sign);
        wxMsg.xml     = GetStringByStrAddr(reg + g_WxCalls.recvMsg.msgXml);

        string roomid = GetStringByWstrAddr(reg + g_WxCalls.recvMsg.roomId);
        if (roomid.find("@chatroom") != string::npos) { // 群 ID 的格式为 xxxxxxxxxxx@chatroom
            wxMsg.is_group = true;
            wxMsg.roomid   = roomid;
            if (wxMsg.is_self) {
                wxMsg.sender = GetSelfWxid();
            } else {
                wxMsg.sender = GetStringByStrAddr(reg + g_WxCalls.recvMsg.wxid);
            }
        } else {
            wxMsg.is_group = false;
            if (wxMsg.is_self) {
                wxMsg.sender = GetSelfWxid();
            } else {
                wxMsg.sender = roomid;
            }
        }

        wxMsg.thumb = GetStringByStrAddr(reg + g_WxCalls.recvMsg.thumb);
        if (!wxMsg.thumb.empty()) {
            wxMsg.thumb = GetHomePath() + wxMsg.thumb;
            replace(wxMsg.thumb.begin(), wxMsg.thumb.end(), '\\', '/');
        }

        wxMsg.extra = GetStringByStrAddr(reg + g_WxCalls.recvMsg.extra);
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
}

__declspec(naked) void RecieveMsgFunc()
{
    __asm {
        pushad
        pushfd
        push ecx
        call DispatchMsg
        add esp, 0x4
        popfd
        popad
        call recvMsgCallAddr // 这个为被覆盖的call
        jmp recvMsgJumpBackAddr // 跳回被HOOK指令的下一条指令
    }
}

void ListenMessage()
{
    // DbgMsg("ListenMessage");
    // OutputDebugString(L"ListenMessage\n");
    // MessageBox(NULL, L"ListenMessage", L"ListenMessage", 0);
    if (gIsListening || (g_WeChatWinDllAddr == 0)) {
        return;
    }

    recvMsgHookAddr     = g_WeChatWinDllAddr + g_WxCalls.recvMsg.hook;
    recvMsgCallAddr     = g_WeChatWinDllAddr + g_WxCalls.recvMsg.call;
    recvMsgJumpBackAddr = recvMsgHookAddr + 5;

    HookAddress(recvMsgHookAddr, RecieveMsgFunc, recvMsgBackupCode);
    gIsListening = true;
}

void UnListenMessage()
{
    if (!gIsListening) {
        return;
    }
    UnHookAddress(recvMsgHookAddr, recvMsgBackupCode);
    gIsListening = false;
}

void DispatchPyq(DWORD reg)
{
    DWORD startAddr = *(DWORD *)(reg + g_WxCalls.pyq.start);
    DWORD endAddr   = *(DWORD *)(reg + g_WxCalls.pyq.end);

    if (startAddr == 0) {
        return;
    }

    while (startAddr < endAddr) {
        WxMsg_t wxMsg;

        wxMsg.type     = 0x00; // 朋友圈消息
        wxMsg.is_self  = false;
        wxMsg.is_group = false;
        wxMsg.id       = GET_QWORD(startAddr);
        wxMsg.ts       = GET_DWORD(startAddr + g_WxCalls.pyq.ts);
        wxMsg.xml      = GetStringByWstrAddr(startAddr + g_WxCalls.pyq.xml);
        wxMsg.sender   = GetStringByWstrAddr(startAddr + g_WxCalls.pyq.wxid);
        wxMsg.content  = GetStringByWstrAddr(startAddr + g_WxCalls.pyq.content);

        {
            unique_lock<mutex> lock(gMutex);
            gMsgQueue.push(wxMsg); // 推送到队列
        }

        gCV.notify_all(); // 通知各方消息就绪

        startAddr += g_WxCalls.pyq.step;
    }
}

__declspec(naked) void RecievePyqFunc()
{
    __asm {
        pushad
        pushfd
        push [esp + 0x24]
        call DispatchPyq
        add esp, 0x4
        popfd
        popad
        call recvPyqCallAddr // 这个为被覆盖的call
        jmp recvPyqJumpBackAddr // 跳回被HOOK指令的下一条指令
    }
}

void ListenPyq()
{
    if (gIsListeningPyq || (g_WeChatWinDllAddr == 0)) {
        return;
    }

    recvPyqHookAddr     = g_WeChatWinDllAddr + g_WxCalls.pyq.hook;
    recvPyqCallAddr     = g_WeChatWinDllAddr + g_WxCalls.pyq.call;
    recvPyqJumpBackAddr = recvPyqHookAddr + 5;

    HookAddress(recvPyqHookAddr, RecievePyqFunc, recvPyqBackupCode);
    gIsListeningPyq = true;
}

void UnListenPyq()
{
    if (!gIsListeningPyq) {
        return;
    }

    UnHookAddress(recvPyqHookAddr, recvPyqBackupCode);
    gIsListeningPyq = false;
}
