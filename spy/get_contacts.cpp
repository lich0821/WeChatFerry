#pragma execution_character_set("utf-8")

#include "get_contacts.h"
#include "load_calls.h"
#include "util.h"

extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

vector<RpcContact_t> GetContacts()
{
    vector<RpcContact_t> contacts;
    DWORD baseAddr = g_WeChatWinDllAddr + g_WxCalls.contact.base;
    DWORD tempAddr = GET_DWORD(baseAddr);
    DWORD head     = GET_DWORD(tempAddr + g_WxCalls.contact.head);
    DWORD node     = GET_DWORD(head);

    while (node != head) {
        RpcContact_t cnt;
        cnt.wxid     = GetStringByAddress(node + g_WxCalls.contact.wxId);
        cnt.code     = GetStringByAddress(node + g_WxCalls.contact.wxCode);
        cnt.remark   = GetStringByAddress(node + g_WxCalls.contact.wxRemark);
        cnt.name     = GetStringByAddress(node + g_WxCalls.contact.wxName);
        cnt.country  = GetStringByAddress(node + g_WxCalls.contact.wxCountry);
        cnt.province = GetStringByAddress(node + g_WxCalls.contact.wxProvince);
        cnt.city     = GetStringByAddress(node + g_WxCalls.contact.wxCity);
        cnt.gender   = GET_DWORD(node + g_WxCalls.contact.wxGender);
        contacts.push_back(cnt);
        node = GET_DWORD(node);
    }

    return contacts;
}
