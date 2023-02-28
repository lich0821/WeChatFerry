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
    char buffer[0x3B0] = { 0 };
    WxString_t wxMsg  = { 0 };
    WxString_t wxWxid = { 0 };

    // 发送消息Call地址 = 微信基址 + 偏移
    DWORD sendCallAddress = g_WeChatWinDllAddr + g_WxCalls.sendTextMsg;

    wstring wsWxid = String2Wstring(wxid);
    wstring wsMsg  = String2Wstring(msg);

    wxMsg.text     = (wchar_t *)wsMsg.c_str();
    wxMsg.size     = wsMsg.size();
    wxMsg.capacity = wsMsg.capacity();

    wxWxid.text     = (wchar_t *)wsWxid.c_str();
    wxWxid.size     = wsWxid.size();
    wxWxid.capacity = wsWxid.capacity();

    vector<WxString_t> vTxtAtWxids;
    if (!atWxids.empty()) {
        vector<wstring> vAtWxids;
        wstringstream wss(String2Wstring(atWxids));
        while (wss.good()) {
            wstring wstr;
            getline(wss, wstr, L',');
            vAtWxids.push_back(wstr);
            WxString_t txtAtWxid = { 0 };
            txtAtWxid.text       = (wchar_t *)vAtWxids.back().c_str();
            txtAtWxid.size       = vAtWxids.back().size();
            txtAtWxid.capacity   = vAtWxids.back().capacity();
            vTxtAtWxids.push_back(txtAtWxid);
        }
    }

    __asm
    {
        lea eax, vTxtAtWxids;
        push 0x01;
        push eax;
        lea edi, wxMsg;
        push edi;
        lea edx, wxWxid;
        lea ecx, buffer;
        call sendCallAddress;
        add esp, 0xC;
    }
}

void SendImageMessage(string wxid, string path)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }
    DWORD tmpEAX       = 0;
    char buf1[0x48]    = { 0 };
    char buf2[0x3B0]   = { 0 };
    WxString_t imgWxid = { 0 };
    WxString_t imgPath = { 0 };

    wstring wsWxid = String2Wstring(wxid);
    wstring wspath = String2Wstring(path);

    imgWxid.text     = (wchar_t *)wsWxid.c_str();
    imgWxid.size     = wsWxid.size();
    imgWxid.capacity = wsWxid.capacity();

    imgPath.text     = (wchar_t *)wspath.c_str();
    imgPath.size     = wspath.size();
    imgPath.capacity = wspath.capacity();

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
        mov ecx, dword ptr[tmpEAX]
        lea eax, imgWxid
        push edi
        push eax
        lea eax, buf2
        push eax
        call sendCall3
        popad
    }
}

void SendFileMessage(string wxid, string path)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }
    DWORD tmpEAX          = 0;
    char buffer[0x3B0]    = { 0 };
    WxString_t fileWxid   = { 0 };
    WxString_t filePath   = { 0 };
    WxString_t nullbuffer = { 0 };

    wstring wsWxid = String2Wstring(wxid);
    wstring wspath = String2Wstring(path);

    fileWxid.text     = (wchar_t *)wsWxid.c_str();
    fileWxid.size     = wsWxid.size();
    fileWxid.capacity = wsWxid.capacity();

    filePath.text     = (wchar_t *)wspath.c_str();
    filePath.size     = wspath.size();
    filePath.capacity = wspath.capacity();

    // 发送文件Call地址 = 微信基址 + 偏移
    DWORD sendCall1 = g_WeChatWinDllAddr + g_WxCalls.sendFile.call1;
    DWORD sendCall2 = g_WeChatWinDllAddr + g_WxCalls.sendFile.call2;
    DWORD sendCall3 = g_WeChatWinDllAddr + g_WxCalls.sendFile.call3;

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
		push 0x00DBE200;
		sub esp, 0x14;
		mov edi, esp;
		mov dword ptr ds : [edi] , 0x0;
		mov dword ptr ds : [edi + 0x4] , 0x0;
		mov dword ptr ds : [edi + 0x8] , 0x0;
		mov dword ptr ds : [edi + 0xC] , 0x0;
		mov dword ptr ds : [edi + 0x10] , 0x0;
		sub esp, 0x14;
		lea eax, filePath;
		mov ecx, esp;
		push eax;
		call sendCall2;
		sub esp, 0x14;
		lea eax, fileWxid;
		mov ecx, esp;
		push eax;
		call sendCall2;
		mov ecx, dword ptr [tmpEAX];
		lea eax, buffer;
		push eax;
		call sendCall3;
		mov al,byte ptr [eax + 0x38];
		movzx eax,al;
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

    char buffer[0xFF0]    = { 0 };
    char nullBuf[0x1C]    = { 0 };
    WxString_t wxReceiver = { 0 };
    WxString_t wxXml      = { 0 };
    WxString_t wxPath     = { 0 };
    WxString_t wxNull     = { 0 };
    WxString_t wxSender   = { 0 };

    wstring wsSender   = String2Wstring(GetSelfWxid());
    wstring wsReceiver = String2Wstring(receiver);
    wstring wsXml      = String2Wstring(xml);

    wxReceiver.text     = (wchar_t *)wsReceiver.c_str();
    wxReceiver.size     = wsReceiver.size();
    wxReceiver.capacity = wsReceiver.capacity();

    wxXml.text     = (wchar_t *)wsXml.c_str();
    wxXml.size     = wsXml.size();
    wxXml.capacity = wsXml.capacity();

    wxSender.text     = (wchar_t *)wsSender.c_str();
    wxSender.size     = wsSender.size();
    wxSender.capacity = wsSender.capacity();

    if (!path.empty()) {
        wstring wsPath  = String2Wstring(path);
        wxPath.text     = (wchar_t *)wsPath.c_str();
        wxPath.size     = wsPath.size();
        wxPath.capacity = wsPath.capacity();
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
