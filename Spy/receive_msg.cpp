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
RpcMessage_t *pMsg        = NULL; // Find a palce to free

void DispatchMsg(DWORD reg)
{
    DWORD *p = (DWORD *)reg; //消息结构基址

    memset(pMsg, 0, sizeof(RpcMessage_t));

    pMsg->type = GET_DWORD(*p + g_WxCalls.recvMsg.type);
    pMsg->self = GET_DWORD(*p + g_WxCalls.recvMsg.isSelf);

    GetWstringByAddress(*p + g_WxCalls.recvMsg.msgId, pMsg->id, MSG_SIZE_MSG_ID);
    GetWstringByAddress(*p + g_WxCalls.recvMsg.msgXml, pMsg->xml, MSG_SIZE_MSG_XML);

    if (wcsstr(pMsg->xml, L"<membercount>") == NULL) {
        // pMsg.roomId = {0};
        GetWstringByAddress(*p + g_WxCalls.recvMsg.roomId, pMsg->wxId, MSG_SIZE_WXID);
    } else {
        pMsg->source = 1;
        GetWstringByAddress(*p + g_WxCalls.recvMsg.roomId, pMsg->roomId, MSG_SIZE_ROOMID);
        GetWstringByAddress(*p + g_WxCalls.recvMsg.wxId, pMsg->wxId, MSG_SIZE_WXID);
    }
    GetWstringByAddress(*p + g_WxCalls.recvMsg.content, pMsg->content, MSG_SIZE_CONTENT);
    g_MsgQueue.push(*pMsg); // 发送消息
    SetEvent(g_hEvent);     // 发送消息通知
}

__declspec(naked) void RecieveMsgHook()
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
    if (g_WeChatWinDllAddr == 0) {
        return;
    }

    pMsg                = new RpcMessage_t;
    DWORD hookAddress   = g_WeChatWinDllAddr + g_WxCalls.recvMsg.hook;
    recvMsgCallAddr     = g_WeChatWinDllAddr + g_WxCalls.recvMsg.call;
    recvMsgJumpBackAddr = hookAddress + 5;

    BYTE jmpCode[5] = { 0 };
    jmpCode[0]      = 0xE9;

    *(DWORD *)&jmpCode[1] = (DWORD)RecieveMsgHook - hookAddress - 5;

    // 6FB6A350 E8 4B020000 call WeChatWi .6FB6A5A0;
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, jmpCode, 5, 0);
}
