#pragma execution_character_set("utf-8")

#include "contact_mgmt.h"
#include "load_calls.h"
#include "log.h"
#include "util.h"

using namespace std;
extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

#define FEAT_LEN 5
static const uint8_t FEAT_COUNTRY[FEAT_LEN]  = { 0xA4, 0xD9, 0x02, 0x4A, 0x18 };
static const uint8_t FEAT_PROVINCE[FEAT_LEN] = { 0xE2, 0xEA, 0xA8, 0xD1, 0x18 };
static const uint8_t FEAT_CITY[FEAT_LEN]     = { 0x1D, 0x02, 0x5B, 0xBF, 0x18 };

static DWORD FindMem(DWORD start, DWORD end, const void *target, size_t len)
{
    uint8_t *p = (uint8_t *)start;
    while ((DWORD)p < end) {
        if (memcmp((void *)p, target, len) == 0) {
            return (DWORD)p;
        }
        p++;
    }

    return 0;
}

static string GetCntString(DWORD start, DWORD end, const uint8_t *feat, size_t len)
{
    DWORD pfeat = FindMem(start, end, feat, len);
    if (pfeat == 0) {
        return "";
    }

    DWORD lfeat = GET_DWORD(pfeat + len);
    if (lfeat <= 2) {
        return "";
    }

    return Wstring2String(wstring(GET_WSTRING_FROM_P(pfeat + FEAT_LEN + 4), lfeat));
}

vector<RpcContact_t> GetContacts()
{
    vector<RpcContact_t> contacts;
    DWORD call1 = g_WeChatWinDllAddr + g_WxCalls.contact.base;
    DWORD call2 = g_WeChatWinDllAddr + g_WxCalls.contact.head;

    int success    = 0;
    DWORD *addr[3] = { 0, 0, 0 };
    __asm {
        pushad
        call       call1
        lea        ecx,addr
        push       ecx
        mov        ecx,eax
        call       call2
        mov        success,eax
        popad
    }

    DWORD pstart = (DWORD)addr[0];
    DWORD pend   = (DWORD)addr[2];

    while (pstart < pend) {
        RpcContact_t cnt;
        DWORD pbin   = GET_DWORD(pstart + 0x150);
        DWORD lenbin = GET_DWORD(pstart + 0x154);

        cnt.wxid   = GetStringByAddress(pstart + g_WxCalls.contact.wxId);
        cnt.code   = GetStringByAddress(pstart + g_WxCalls.contact.wxCode);
        cnt.remark = GetStringByAddress(pstart + g_WxCalls.contact.wxRemark);
        cnt.name   = GetStringByAddress(pstart + g_WxCalls.contact.wxName);

        cnt.country  = GetCntString(pbin, pbin + lenbin, FEAT_COUNTRY, FEAT_LEN);
        cnt.province = GetCntString(pbin, pbin + lenbin, FEAT_PROVINCE, FEAT_LEN);
        cnt.city     = GetCntString(pbin, pbin + lenbin, FEAT_CITY, FEAT_LEN);

        if (pbin == 0) {
            cnt.gender = 0;
        } else {
            cnt.gender = (DWORD) * (uint8_t *)(pbin + g_WxCalls.contact.wxGender);
        }

        contacts.push_back(cnt);
        pstart += 0x438;
    }

    return contacts;
}

int AcceptNewFriend(string v3, string v4, int scene)
{
    int success = 0;

    DWORD acceptNewFriendCall1 = g_WeChatWinDllAddr + g_WxCalls.anf.call1;
    DWORD acceptNewFriendCall2 = g_WeChatWinDllAddr + g_WxCalls.anf.call2;
    DWORD acceptNewFriendCall3 = g_WeChatWinDllAddr + g_WxCalls.anf.call3;
    DWORD acceptNewFriendCall4 = g_WeChatWinDllAddr + g_WxCalls.anf.call4;

    char buffer[0x40]      = { 0 };
    char nullbuffer[0x3CC] = { 0 };

    LOG_DEBUG("\nv3: {}\nv4: {}\nscene: {}", v3, v4, scene);

    wstring wsV3 = String2Wstring(v3);
    wstring wsV4 = String2Wstring(v4);
    WxString wxV3(wsV3);
    WxString wxV4(wsV4);

    __asm {
        pushad;
        pushfd;
        lea ecx, buffer;
        call acceptNewFriendCall1;
        mov esi, 0x0;
        mov edi, scene;
        push esi;
        push edi;
        sub esp, 0x14;
        mov ecx, esp;
        lea eax, wxV4;
        push eax;
        call acceptNewFriendCall2;
        sub esp, 0x8;
        push 0x0;
        lea eax, nullbuffer;
        push eax;
        lea eax, wxV3;
        push eax;
        lea ecx, buffer;
        call acceptNewFriendCall3;
        mov success, eax;
        lea ecx, buffer;
        call acceptNewFriendCall4;
        popfd;
        popad;
    }

    return success; // 成功返回 1
}

/*没啥用，非好友获取不到*/
RpcContact_t GetContactByWxid(string wxid)
{
    RpcContact_t contact;
    char buff[0x440] = { 0 };
    wstring wsWxid   = String2Wstring(wxid);
    WxString pri(wsWxid);
    DWORD contact_mgr_addr  = g_WeChatWinDllAddr + 0x75A4A0;
    DWORD get_contact_addr  = g_WeChatWinDllAddr + 0xC04E00;
    DWORD free_contact_addr = g_WeChatWinDllAddr + 0xEA7880;
    __asm {
        PUSHAD
        PUSHFD
        CALL       contact_mgr_addr
        LEA        ECX,buff
        PUSH       ECX
        LEA        ECX,pri
        PUSH       ECX
        MOV        ECX,EAX
        CALL       get_contact_addr
        POPFD
        POPAD
    }

    contact.wxid   = wxid;
    contact.code   = GetStringByWstrAddr((DWORD)buff + g_WxCalls.contact.wxCode);
    contact.remark = GetStringByWstrAddr((DWORD)buff + g_WxCalls.contact.wxRemark);
    contact.name   = GetStringByWstrAddr((DWORD)buff + g_WxCalls.contact.wxName);
    contact.gender = GET_DWORD((DWORD)buff + 0x148);

    __asm {
        PUSHAD
        PUSHFD
        LEA        ECX,buff
        CALL       free_contact_addr
        POPFD
        POPAD
    }

    return contact;
}
