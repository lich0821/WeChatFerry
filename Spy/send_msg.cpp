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

void SendImageMessage(const wchar_t *wxid, const wchar_t *path)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }
    DWORD tmpEAX         = 0;
    char buf1[0x48]      = { 0 };
    char buf2[0x3B0]     = { 0 };
    TextStruct_t imgWxid = { 0 };
    TextStruct_t imgPath = { 0 };

    wstring wsWxid = wxid;
    wstring wsPath = path;

    imgWxid.text     = (wchar_t *)wsWxid.c_str();
    imgWxid.size     = wsWxid.size();
    imgWxid.capacity = wsWxid.capacity();

    imgPath.text     = (wchar_t *)wsPath.c_str();
    imgPath.size     = wsPath.size();
    imgPath.capacity = wsPath.capacity();

    // 发送图片Call地址 = 微信基址 + 偏移
    DWORD sendCall1 = g_WeChatWinDllAddr + g_WxCalls.sendImg.call1;
    DWORD sendCall2 = g_WeChatWinDllAddr + g_WxCalls.sendImg.call2;
    DWORD sendCall3 = g_WeChatWinDllAddr + g_WxCalls.sendImg.call3;

    __asm {
        pushad
        call sendCall1
        sub esp, 0x14
        mov tmpEAX, eax
        lea eax, buf1
        mov ecx, esp
        lea edi, imgPath
        push eax
        call sendCall2
        mov ecx, dword ptr [tmpEAX]
        lea eax, imgWxid
        push edi
        push eax
        lea eax, buf2
        push eax
        call sendCall3
        popad
    }
}
