#include "get_contacts.h"
#include "load_calls.h"
#include "util.h"

extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

std::vector<RpcContact_t> GetContacts()
{
    int gender = 0;
    vector<RpcContact_t> vContacts;
    DWORD baseAddr = g_WeChatWinDllAddr + 0x23638F4;
    DWORD tempAddr = GET_DWORD(baseAddr);
    DWORD head     = GET_DWORD(tempAddr + 0x4C);
    DWORD node     = GET_DWORD(head);

    while (node != head) {
        RpcContact_t rpcContact = { 0 };
        rpcContact.wxId         = GetBstrByAddress(node + 0x30);
        rpcContact.wxCode       = GetBstrByAddress(node + 0x44);
        rpcContact.wxName       = GetBstrByAddress(node + 0x8C);
        rpcContact.wxCountry    = GetBstrByAddress(node + 0x1D0);
        rpcContact.wxProvince   = GetBstrByAddress(node + 0x1E4);
        rpcContact.wxCity       = GetBstrByAddress(node + 0x1F8);

        gender = GET_DWORD(node + 0x184);
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
