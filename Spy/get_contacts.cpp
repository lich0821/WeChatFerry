#include "get_contacts.h"
#include "load_calls.h"
#include "util.h"

extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

std::vector<RpcContact_t> GetContacts()
{
    int gender = 0;
    vector<RpcContact_t> vContacts;
    DWORD baseAddr = g_WeChatWinDllAddr + g_WxCalls.contact.base;
    DWORD tempAddr = GET_DWORD(baseAddr);
    DWORD head     = GET_DWORD(tempAddr + g_WxCalls.contact.head);
    DWORD node     = GET_DWORD(head);

    while (node != head) {
        RpcContact_t rpcContact = { 0 };
        rpcContact.wxId         = GetBstrByAddress(node + g_WxCalls.contact.wxId);
        rpcContact.wxCode       = GetBstrByAddress(node + g_WxCalls.contact.wxCode);
        rpcContact.wxName       = GetBstrByAddress(node + g_WxCalls.contact.wxName);
        rpcContact.wxCountry    = GetBstrByAddress(node + g_WxCalls.contact.wxCountry);
        rpcContact.wxProvince   = GetBstrByAddress(node + g_WxCalls.contact.wxProvince);
        rpcContact.wxCity       = GetBstrByAddress(node + g_WxCalls.contact.wxCity);

        gender = GET_DWORD(node + g_WxCalls.contact.wxGender);
        if (gender == 1)
            rpcContact.wxGender = SysAllocString(L"男");
        else if (gender == 2)
            rpcContact.wxGender = SysAllocString(L"女");
        else
            rpcContact.wxGender = SysAllocString(L"未知");

        vContacts.push_back(rpcContact);
        node = GET_DWORD(node);
    }

    return vContacts;
}
