#include "framework.h"
#include <sstream>
#include <vector>

#include "send_msg.h"
#include "spy_types.h"
#include "util.h"

extern HANDLE g_hEvent;
extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

typedef struct AtList {
    DWORD start;
    DWORD end1;
    DWORD end2;
} AtList_t;

void SendTextMessage(string wxid, string msg, string atWxids)
{
    char buffer[0x3B0]    = { 0 };
    AtList_t atList       = { 0 };
    TextStruct_t txtMsg   = { 0 };
    TextStruct_t txtWxid  = { 0 };
    TextStruct_t *tsArray = NULL;

    // 发送消息Call地址 = 微信基址 + 偏移
    DWORD sendCallAddress = g_WeChatWinDllAddr + g_WxCalls.sendTextMsg;

    wstring wsWxid = String2Wstring(wxid);
    wstring wsMsg  = String2Wstring(msg);

    txtMsg.text     = (wchar_t *)wsMsg.c_str();
    txtMsg.size     = wsMsg.size();
    txtMsg.capacity = wsMsg.capacity();

    txtWxid.text     = (wchar_t *)wsWxid.c_str();
    txtWxid.size     = wsWxid.size();
    txtWxid.capacity = wsWxid.capacity();

    if (!atWxids.empty()) {
        int i       = 0;
        wstring tmp = String2Wstring(atWxids);
        wstring wstr;
        vector<wstring> vAtWxids;
        wstringstream wss(tmp);
        while (wss.good()) {
            getline(wss, wstr, L',');
            vAtWxids.push_back(wstr);
        }
        tsArray = new TextStruct_t[vAtWxids.size() + 1];
        // memset(tsArray, 0, (vAtWxids.size() + 1) * sizeof(TextStruct_t));
        for (auto it = vAtWxids.begin(); it != vAtWxids.end(); it++) {
            tsArray[i].text     = (wchar_t *)it->c_str();
            tsArray[i].size     = it->size();
            tsArray[i].capacity = it->capacity();
            i++;
        }

        atList.start = (DWORD)tsArray;
        atList.end1  = (DWORD)&tsArray[i];
        atList.end2  = (DWORD)&tsArray[i];
    }

    __asm
    {
        lea eax, atList;
        push 0x01;
        push eax;
        lea edi, txtMsg;
        push edi;
        lea edx, txtWxid;
        lea ecx, buffer;
        call sendCallAddress;
        add esp, 0xC;
    }

    if (tsArray)
    {
        delete[] tsArray;
        tsArray = NULL;
    }
}

void SendImageMessage(string wxid, string path)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }
    DWORD tmpEAX         = 0;
    char buf1[0x48]      = { 0 };
    char buf2[0x3B0]     = { 0 };
    TextStruct_t imgWxid = { 0 };
    TextStruct_t imgPath = { 0 };

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
    DWORD tmpEAX            = 0;
    char buffer[0x3B0]      = { 0 };
    TextStruct_t fileWxid   = { 0 };
    TextStruct_t filePath   = { 0 };
    TextStruct_t nullbuffer = { 0 };

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
