#include "framework.h"
#include <sstream>
#include <vector>

#include "send_msg.h"
#include "spy_types.h"
#include "util.h"

extern HANDLE g_hEvent;
extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;
extern string GetSelfWxid(); // Defined in spy.cpp

void SendTextMessage(string wxid, string msg, string atWxids)
{
    int success        = 0;
    char buffer[0x2D8] = { 0 };

    // 发送消息Call地址 = 微信基址 + 偏移
    DWORD sendCall1 = g_WeChatWinDllAddr + g_WxCalls.sendText.call1;
    DWORD sendCall2 = g_WeChatWinDllAddr + g_WxCalls.sendText.call2;
    DWORD sendCall3 = g_WeChatWinDllAddr + g_WxCalls.sendText.call3;

    wstring wsWxid = String2Wstring(wxid);
    wstring wsMsg  = String2Wstring(msg);
    WxString wxMsg(wsMsg);
    WxString wxWxid(wsWxid);

    vector<WxString> vWxAtWxids;
    if (!atWxids.empty()) {
        vector<wstring> vAtWxids;
        wstringstream wss(String2Wstring(atWxids));
        while (wss.good()) {
            wstring wstr;
            getline(wss, wstr, L',');
            vAtWxids.push_back(wstr);
            WxString wxAtWxid(vAtWxids.back());
            vWxAtWxids.push_back(wxAtWxid);
        }
    }

    __asm
    {
        pushad;
        call sendCall1;
        push 0x0;
        push 0x0;
        push 0x0;
        push 0x1;
        lea eax, vWxAtWxids;
        push eax;
        lea eax, wxMsg;
        push eax;
        lea edx, wxWxid;
        lea ecx, buffer;
        call sendCall2;
        mov success, eax;
        add esp, 0x18;
        lea ecx, buffer;
        call sendCall3;
        popad;
    }
}

void SendImageMessage(string wxid, string path)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }
    int success     = 0;
    DWORD tmpEAX    = 0;
    char buf[0x2D8] = { 0 };

    wstring wsWxid = String2Wstring(wxid);
    wstring wsPath = String2Wstring(path);

    WxString wxWxid(wsWxid);
    WxString wxPath(wsPath);
    WxString nullbuffer;

    // 发送图片Call地址 = 微信基址 + 偏移
    DWORD sendCall1 = g_WeChatWinDllAddr + g_WxCalls.sendImg.call1;
    DWORD sendCall2 = g_WeChatWinDllAddr + g_WxCalls.sendImg.call2;
    DWORD sendCall3 = g_WeChatWinDllAddr + g_WxCalls.sendImg.call3;
    DWORD sendCall4 = g_WeChatWinDllAddr + g_WxCalls.sendImg.call4;

    __asm {
        pushad;
        call       sendCall1;
        sub        esp,0x14;
        mov        tmpEAX,eax;
        lea        eax,nullbuffer;
        mov        ecx,esp;
        lea        edi,wxPath;
        push       eax;
        call       sendCall2;
        mov        ecx,dword ptr [tmpEAX];
        lea        eax,wxWxid;
        push       edi;
        push       eax;
        lea        eax,buf;
        push       eax;
        call       sendCall3;
        mov        success,eax;
        lea        ecx,buf;
        call       sendCall4;
        popad;
    }
}

void SendFileMessage(string wxid, string path)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }
    int success        = 0;
    DWORD tmpEAX       = 0;
    char buffer[0x2D8] = { 0 };

    wstring wsWxid = String2Wstring(wxid);
    wstring wsPath = String2Wstring(path);

    WxString wxWxid(wsWxid);
    WxString wxPath(wsPath);
    WxString nullbuffer;

    // 发送文件Call地址 = 微信基址 + 偏移
    DWORD sendCall1 = g_WeChatWinDllAddr + g_WxCalls.sendFile.call1;
    DWORD sendCall2 = g_WeChatWinDllAddr + g_WxCalls.sendFile.call2;
    DWORD sendCall3 = g_WeChatWinDllAddr + g_WxCalls.sendFile.call3;
    DWORD sendCall4 = g_WeChatWinDllAddr + g_WxCalls.sendFile.call4;

    __asm {
        pushad;
        pushfd;
        call sendCall1;
        sub esp, 0x14;
        mov tmpEAX, eax;
        lea eax, nullbuffer;
        mov ecx, esp;
        push eax;
        call sendCall2;
        push 0x0;
        sub esp, 0x14;
        mov edi, esp;
        mov dword ptr[edi], 0;
        mov dword ptr[edi + 0x4], 0;
        mov dword ptr[edi + 0x8], 0;
        mov dword ptr[edi + 0xc], 0;
        mov dword ptr[edi + 0x10], 0;
        sub esp, 0x14;
        lea eax, wxPath;
        mov ecx, esp;
        push eax;
        call sendCall2;
        sub esp, 0x14;
        lea eax, wxWxid;
        mov ecx, esp;
        push eax;
        call sendCall2;
        mov ecx, dword ptr[tmpEAX];
        lea eax, buffer;
        push eax;
        call sendCall3;
        mov al, byte ptr[eax + 0x38];
        movzx eax, al;
        mov success, eax;
        lea ecx, buffer;
        call sendCall4;
        popfd;
        popad;
    }
}
void SendXmlMessage(string receiver, string xml, string path, int type)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }

    // 发送消息Call地址 = 微信基址 + 偏移
    DWORD sendXmlCall1 = g_WeChatWinDllAddr + g_WxCalls.sendXml.call1;
    DWORD sendXmlCall2 = g_WeChatWinDllAddr + g_WxCalls.sendXml.call2;
    DWORD sendXmlCall3 = g_WeChatWinDllAddr + g_WxCalls.sendXml.call3;
    DWORD sendXmlCall4 = g_WeChatWinDllAddr + g_WxCalls.sendXml.call4;
    DWORD sendXmlParam = g_WeChatWinDllAddr + g_WxCalls.sendXml.param;

    char buffer[0xFF0] = { 0 };
    char nullBuf[0x1C] = { 0 };

    wstring wsSender   = String2Wstring(GetSelfWxid());
    wstring wsReceiver = String2Wstring(receiver);
    wstring wsXml      = String2Wstring(xml);

    WxString wxPath;
    WxString wxNull;
    WxString wxXml(wsXml);
    WxString wxSender(wsSender);
    WxString wxReceiver(wsReceiver);

    if (!path.empty()) {
        wstring wsPath = String2Wstring(path);
        wxPath         = WxString(wsPath);
    }

    DWORD sendtype = type;
    __asm {
		pushad;
		pushfd;
		lea ecx, buffer;
		call sendXmlCall1;
		mov eax, [sendtype];
		push eax;
		lea eax, nullBuf;
		lea edx, wxSender;
		push eax;
		lea eax, wxPath;
		push eax;
		lea eax, wxXml;
		push eax;
		lea edi, wxReceiver;
		push edi;
		lea ecx, buffer;
		call sendXmlCall2;
		add esp, 0x14;
		lea eax, wxNull;
		push eax;
		lea ecx, buffer;
		call sendXmlCall3;
		mov dl, 0x0;
		lea ecx, buffer;
		push sendXmlParam;
		push sendXmlParam;
		call sendXmlCall4;
		add esp, 0x8;
		popfd;
		popad;
    }
}

void SendEmotionMessage(string wxid, string path)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }

    char buffer[0x1C] = { 0 };
    wstring wsWxid    = String2Wstring(wxid);
    wstring wsPath    = String2Wstring(path);

    WxString wxWxid(wsWxid);
    WxString wxPath(wsPath);
    WxString nullbuffer;

    // 发送文件Call地址 = 微信基址 + 偏移
    DWORD sendCall1 = g_WeChatWinDllAddr + g_WxCalls.sendEmo.call1;
    DWORD sendCall2 = g_WeChatWinDllAddr + g_WxCalls.sendEmo.call2;
    DWORD sendCall3 = g_WeChatWinDllAddr + g_WxCalls.sendEmo.call3;

    __asm {
        pushad;
        pushfd;
        mov ebx, dword ptr[sendCall3];
        lea eax, buffer;
        push eax;
        push 0x0;
        sub esp, 0x14;
        mov esi, esp;
        mov dword ptr [esi], 0x0;
        mov dword ptr [esi+0x4], 0x0;
        mov dword ptr [esi+0x8], 0x0;
        mov dword ptr [esi+0xC], 0x0;
        mov dword ptr [esi+0x10], 0x0;
        push 0x2;
        lea eax, wxWxid;
        sub esp, 0x14;
        mov ecx, esp;
        push eax;
        call sendCall1;
        sub esp, 0x14;
        mov esi, esp;
        mov dword ptr [esi], 0x0;
        mov dword ptr [esi+0x4], 0x0;
        mov dword ptr [esi+0x8], 0x0;
        mov dword ptr [esi+0xC], 0x0;
        mov dword ptr [esi+0x10], 0x0;
        sub esp, 0x14;
        mov ecx, esp;
        lea eax, wxPath;
        push eax;
        call sendCall1;
        mov ecx, ebx;
        call sendCall2;
        popfd;
        popad;
    }
}
