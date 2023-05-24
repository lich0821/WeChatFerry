#pragma execution_character_set("utf-8")

#include "get_contacts.h"
#include "load_calls.h"
#include "util.h"

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

    // DbgMsg("pfeat: %08X, lfeat: %d", pfeat, lfeat);
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
        // DbgMsg("pstart: %08X, pbin: %08X, lenbin: %d", pstart, pbin, lenbin);

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
        // DbgMsg("pstart: %08X, pbin: %08X, lenbin: %d, cnt.gender: %08X", pstart, pbin, lenbin, cnt.gender);

        contacts.push_back(cnt);
        pstart += 0x438;
    }

    return contacts;
}
