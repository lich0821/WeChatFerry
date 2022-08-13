#include "framework.h"

#include "load_calls.h"
#include "receive_msg.h"
#include "spy_types.h"
#include "util.h"

extern HANDLE g_hEvent;
extern RpcMessage_t *g_pMsg;
extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

MsgQueue_t g_MsgQueue;
static BOOL isListened           = false;
static DWORD reg_buffer          = 0;
static DWORD recvMsgHookAddr     = 0;
static DWORD recvMsgCallAddr     = 0;
static DWORD recvMsgJumpBackAddr = 0;
static CHAR recvMsgBackupCode[5] = { 0 };
static RpcMessage_t lMsg         = { 0 };

extern const MsgTypesMap_t g_WxMsgTypes = MsgTypesMap_t { { 0x01, L"文字" },
                                                          { 0x03, L"图片" },
                                                          { 0x22, L"语音" },
                                                          { 0x25, L"好友确认" },
                                                          { 0x28, L"POSSIBLEFRIEND_MSG" },
                                                          { 0x2A, L"名片" },
                                                          { 0x2B, L"视频" },
                                                          { 0x2F, L"石头剪刀布 | 表情图片" },
                                                          { 0x30, L"位置" },
                                                          { 0x31, L"共享实时位置、文件、转账、链接" },
                                                          { 0x32, L"VOIPMSG" },
                                                          { 0x33, L"微信初始化" },
                                                          { 0x34, L"VOIPNOTIFY" },
                                                          { 0x35, L"VOIPINVITE" },
                                                          { 0x3E, L"小视频" },
                                                          { 0x270F, L"SYSNOTICE" },
                                                          { 0x2710, L"红包、系统消息" },
                                                          { 0x2712, L"撤回消息" } };

void HookAddress(DWORD hookAddr, LPVOID funcAddr, CHAR recvMsgBackupCode[5])
{
    //组装跳转数据
    BYTE jmpCode[5] = { 0 };
    jmpCode[0]      = 0xE9;

    //计算偏移
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
    DWORD *p = (DWORD *)reg; //消息结构基址

    memset(&lMsg, 0, sizeof(RpcMessage_t));

    lMsg.type = GET_DWORD(*p + g_WxCalls.recvMsg.type);
    lMsg.self = GET_DWORD(*p + g_WxCalls.recvMsg.isSelf);
    lMsg.id   = GetBstrByAddress(*p + g_WxCalls.recvMsg.msgId);
    lMsg.xml  = GetBstrByAddress(*p + g_WxCalls.recvMsg.msgXml);

    if (wcsstr(lMsg.xml, L"<membercount>") == NULL) {
        // pMsg.roomId = {0};
        lMsg.wxId = GetBstrByAddress(*p + g_WxCalls.recvMsg.roomId);
    } else {
        lMsg.source = 1;
        lMsg.wxId   = GetBstrByAddress(*p + g_WxCalls.recvMsg.wxId);
        lMsg.roomId = GetBstrByAddress(*p + g_WxCalls.recvMsg.roomId);
    }
    lMsg.content = GetBstrByAddress(*p + g_WxCalls.recvMsg.content);
    g_MsgQueue.push(lMsg); // 发送消息
    SetEvent(g_hEvent);    // 发送消息通知
}

__declspec(naked) void RecieveMsgFunc()
{
    __asm {
        mov reg_buffer, edi //把值复制出来
    }

    DispatchMsg(reg_buffer);

    __asm
    {
        call recvMsgCallAddr    // 这个为被覆盖的call
        jmp recvMsgJumpBackAddr // 跳回被HOOK指令的下一条指令
    }
}

void ListenMessage()
{
    // MessageBox(NULL, L"ListenMessage", L"ListenMessage", 0);
    if (isListened || (g_WeChatWinDllAddr == 0)) {
        return;
    }

    recvMsgHookAddr     = g_WeChatWinDllAddr + g_WxCalls.recvMsg.hook;
    recvMsgCallAddr     = g_WeChatWinDllAddr + g_WxCalls.recvMsg.call;
    recvMsgJumpBackAddr = recvMsgHookAddr + 5;

    HookAddress(recvMsgHookAddr, RecieveMsgFunc, recvMsgBackupCode);
    isListened = true;
}

void UnListenMessage()
{
    if (!isListened) {
        return;
    }
    UnHookAddress(recvMsgHookAddr, recvMsgBackupCode);
    isListened = false;
}
