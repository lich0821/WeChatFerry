#include <iostream>
#include <map>

#include "load_calls.h"

std::map<std::wstring, WxCalls_t> wxCalls {
    { L"3.0.0.57",
      { 0x1874F38,                           // Login Status
        { 0x1856BF0, 0x1856A8C, 0x1856AC0 }, // User Info: wxid, nickname, mobile
        0x38D8A0,                            // Send Message
        // Receive Message:
        // Hook,      call,   type, self,  id, msgXml, roomId, wxId, content
        { 0x36A350, 0x36A5A0, 0x30, 0x34, 0x164, 0x1A4, 0x40, 0x150, 0x68 } } }
};

int LoadCalls(const wchar_t *version, WxCalls_t *calls)
{
    auto iter = wxCalls.find(version);
    if (iter == wxCalls.end()) {
        return -1;
    }

    memcpy_s(calls, sizeof(WxCalls_t), &(iter->second), sizeof(WxCalls_t));

    return 0;
}
