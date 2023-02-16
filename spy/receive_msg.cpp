#pragma execution_character_set("utf-8")

#include "receive_msg.h"

MsgTypes_t GetMsgTypes()
{
    const MsgTypes_t m = { { 0x01, "文字" },
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
                           { 0x270F, "SYSNOTICE" },
                           { 0x2710, "红包、系统消息" },
                           { 0x2712, "撤回消息" } };

    return m;
}

#if 0
#include <queue>

#include "framework.h"

#include "load_calls.h"
#include "receive_msg.h"
#include "spy_types.h"
#include "util.h"

using namespace std;

using wcf::MsgTypes;
using wcf::WxMsg;

extern bool gIsListening;
extern mutex gMutex;
extern queue<WxMsg> gMsgQueue;
extern condition_variable gCv;
extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

static DWORD reg_buffer          = 0;
static DWORD recvMsgHookAddr     = 0;
static DWORD recvMsgCallAddr     = 0;
static DWORD recvMsgJumpBackAddr = 0;
static CHAR recvMsgBackupCode[5] = { 0 };

void GetMsgTypes(MsgTypes *types)
{
    const map<int32_t, string> m = { { 0x01, "文字" },
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
                                     { 0x270F, "SYSNOTICE" },
                                     { 0x2710, "红包、系统消息" },
                                     { 0x2712, "撤回消息" } };

    for (auto &[k, v] : m) {
        (*(types->mutable_types()))[k] = v;
    }
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
    WxMsg wxMsg;
    DWORD *p = (DWORD *)reg; // 消息结构基址

    wxMsg.set_type(GET_DWORD(*p + g_WxCalls.recvMsg.type));
    wxMsg.set_is_self(GET_DWORD(*p + g_WxCalls.recvMsg.isSelf));
    wxMsg.set_id(GetStringByAddress(*p + g_WxCalls.recvMsg.msgId));
    wxMsg.set_xml(GetStringByAddress(*p + g_WxCalls.recvMsg.msgXml));

    // 群里的系统消息，xml 为空
    if ((wxMsg.xml().empty()) || (strstr(wxMsg.xml().c_str(), "<membercount>") != NULL)) {
        wxMsg.set_is_group(true);
        wxMsg.set_sender(GetStringByAddress(*p + g_WxCalls.recvMsg.wxId));
        wxMsg.set_roomid(GetStringByAddress(*p + g_WxCalls.recvMsg.roomId));
    } else {
        wxMsg.set_is_group(false);
        wxMsg.set_sender(GetStringByAddress(*p + g_WxCalls.recvMsg.roomId));
    }
    wxMsg.set_content(GetStringByAddress(*p + g_WxCalls.recvMsg.content));

    // 推送到队列
    unique_lock<std::mutex> lock(gMutex);
    gMsgQueue.push(wxMsg);
    lock.unlock();

    // 通知各方消息就绪
    gCv.notify_all();
}

__declspec(naked) void RecieveMsgFunc()
{
    __asm {
        mov reg_buffer, edi // 把值复制出来
    }

    DispatchMsg(reg_buffer);

    __asm
    {
        call recvMsgCallAddr // 这个为被覆盖的call
        jmp recvMsgJumpBackAddr // 跳回被HOOK指令的下一条指令
    }
}

void ListenMessage()
{
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
#endif