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
DWORD reg_buffer          = 0;
DWORD recvMsgCallAddr     = 0;
DWORD recvMsgJumpBackAddr = 0;

void DispatchMsg(DWORD reg)
{
    DWORD **p = (DWORD **)reg; //消息结构基址

    memset(g_pMsg, 0, sizeof(RpcMessage_t));

    g_pMsg->type = GET_DWORD(**p + g_WxCalls.recvMsg.type);
    g_pMsg->self = GET_DWORD(**p + g_WxCalls.recvMsg.isSelf);

    GetWstringByAddress(**p + g_WxCalls.recvMsg.msgId, g_pMsg->id, MSG_SIZE_MSG_ID);
    GetWstringByAddress(**p + g_WxCalls.recvMsg.msgXml, g_pMsg->xml, MSG_SIZE_MSG_XML);

    if (wcsstr(g_pMsg->xml, L"<membercount>") == NULL) {
        // g_pMsg.roomId = {0};
        GetWstringByAddress(**p + g_WxCalls.recvMsg.roomId, g_pMsg->wxId, MSG_SIZE_WXID);
    } else {
        g_pMsg->source = 1;
        GetWstringByAddress(**p + g_WxCalls.recvMsg.roomId, g_pMsg->roomId, MSG_SIZE_ROOMID);
        GetWstringByAddress(**p + g_WxCalls.recvMsg.wxId, g_pMsg->wxId, MSG_SIZE_WXID);
    }
    GetWstringByAddress(**p + g_WxCalls.recvMsg.content, g_pMsg->content, MSG_SIZE_CONTENT);
    g_MsgQueue.push(*g_pMsg); // 发送消息
    SetEvent(g_hEvent);       // 发送消息通知
}

__declspec(naked) void RecieveMsgHook()
{
    __asm {
        push ebp            // 保护现场
        add ebp, 0x3C       // 地址为 ebp + 0x3C
        mov reg_buffer, ebp //把值复制出来
        pop ebp             // 还原现场
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
    if (g_WeChatWinDllAddr == 0) {
        return;
    }

    DWORD hookAddress   = g_WeChatWinDllAddr + g_WxCalls.recvMsg.hook;
    recvMsgCallAddr     = g_WeChatWinDllAddr + g_WxCalls.recvMsg.call;
    recvMsgJumpBackAddr = hookAddress + 5;

    BYTE jmpCode[5] = { 0 };
    jmpCode[0]      = 0xE9;

    *(DWORD *)&jmpCode[1] = (DWORD)RecieveMsgHook - hookAddress - 5;

    // 6FB6A350 E8 4B020000 call WeChatWi .6FB6A5A0;
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, jmpCode, 5, 0);
}