#include "framework.h"
#include <string>

#include "spy_types.h"

extern HANDLE g_hEvent;
extern WxCalls_t g_WxCalls;
extern RpcMessage_t *g_pMsg;
extern DWORD g_WeChatWinDllAddr;

using namespace std;

void SendTextMessage(const wchar_t *wxid, const wchar_t *at_wxid, const wchar_t *msg)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }
    char buffer[0x5F0]     = { 0 };
    TextStruct_t txtWxid   = { 0 };
    TextStruct_t txtAtWxid = { 0 };
    TextStruct_t txtMsg    = { 0 };

    wstring wsWxid   = wxid;
    wstring wsAtWxid = at_wxid;
    wstring wsMsg    = msg;

    // 发送消息Call地址 = 微信基址 + 偏移
    DWORD sendCallAddress = g_WeChatWinDllAddr + g_WxCalls.sendTextMsg;

    txtWxid.text     = (wchar_t *)wsWxid.c_str();
    txtWxid.size     = wsWxid.size();
    txtWxid.capacity = wsWxid.capacity();

    txtMsg.text     = (wchar_t *)wsMsg.c_str();
    txtMsg.size     = wsMsg.size();
    txtMsg.capacity = wsMsg.capacity();

    __asm {
        lea edx, txtWxid
        lea edi, txtAtWxid
        lea ebx, txtMsg
        push 0x01
        push edi
        push ebx
        lea ecx, buffer;
        call sendCallAddress
            add esp, 0xC
    }
}
