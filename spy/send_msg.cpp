#include <sstream>
#include <string>
#include <vector>

#include "send_msg.h"
#include "spy_types.h"

extern HANDLE g_hEvent;
extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

typedef struct AtList {
    DWORD start;
    DWORD end1;
    DWORD end2;
} AtList_t;

void SendTextMessage(wstring wxid, wstring msg, wstring atWxids)
{
    char buffer[0x3B0]    = { 0 };
    AtList_t atList       = { 0 };
    TextStruct_t txtMsg   = { 0 };
    TextStruct_t txtWxid  = { 0 };
    TextStruct_t *tsArray = NULL;

    // 发送消息Call地址 = 微信基址 + 偏移
    DWORD sendCallAddress = g_WeChatWinDllAddr + g_WxCalls.sendTextMsg;

    txtMsg.text     = (wchar_t *)msg.c_str();
    txtMsg.size     = msg.size();
    txtMsg.capacity = msg.capacity();

    txtWxid.text     = (wchar_t *)wxid.c_str();
    txtWxid.size     = wxid.size();
    txtWxid.capacity = wxid.capacity();

    wstring tmp = atWxids;
    if (!tmp.empty()) {
        int i = 0;
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

void SendImageMessage(wstring wxid, wstring path)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }
    DWORD tmpEAX         = 0;
    char buf1[0x48]      = { 0 };
    char buf2[0x3B0]     = { 0 };
    TextStruct_t imgWxid = { 0 };
    TextStruct_t imgPath = { 0 };

    imgWxid.text     = (wchar_t *)wxid.c_str();
    imgWxid.size     = wxid.size();
    imgWxid.capacity = wxid.capacity();

    imgPath.text     = (wchar_t *)path.c_str();
    imgPath.size     = path.size();
    imgPath.capacity = path.capacity();

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
