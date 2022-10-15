#include "get_contacts.h"
#include "load_calls.h"
#include "util.h"

extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

bool GetContacts(wcf::Contacts* contacts)
{
    int gender = 0;
    DWORD baseAddr = g_WeChatWinDllAddr + g_WxCalls.contact.base;
    DWORD tempAddr = GET_DWORD(baseAddr);
    DWORD head     = GET_DWORD(tempAddr + g_WxCalls.contact.head);
    DWORD node     = GET_DWORD(head);

    while (node != head) {
        wcf::Contact* cnt = contacts->add_contacts();
        cnt->set_wxid(GetStringByAddress(node + g_WxCalls.contact.wxId));
        cnt->set_code(GetStringByAddress(node + g_WxCalls.contact.wxCode));
        cnt->set_name(GetStringByAddress(node + g_WxCalls.contact.wxName));
        cnt->set_country(GetStringByAddress(node + g_WxCalls.contact.wxCountry));
        cnt->set_province(GetStringByAddress(node + g_WxCalls.contact.wxProvince));
        cnt->set_city(GetStringByAddress(node + g_WxCalls.contact.wxCity));
        cnt->set_city(GetStringByAddress(node + g_WxCalls.contact.wxCity));

        gender = GET_DWORD(node + g_WxCalls.contact.wxGender);
        if (gender == 1)
            cnt->set_city("男");
        else if (gender == 2)
            cnt->set_city("女");
        else
            cnt->set_city("未知");

        node = GET_DWORD(node);
    }

    return true;
}
