#include <iostream>
#include <map>

#include "load_calls.h"

#define SUPPORT_VERSION L"3.0.0.57"
WxCalls_t wxCalls = { 0x1874F38,                           // Login Status
                      { 0x1856BF0, 0x1856A8C, 0x1856AC0 }, // User Info: wxid, nickname, mobile
                      0x38D8A0,                            // Send Message
                      // Receive Message:
                      // Hook,      call,   type, self,  id, msgXml, roomId, wxId, content
                      { 0x36A350, 0x36A5A0, 0x30, 0x34, 0x164, 0x1A4, 0x40, 0x150, 0x68 } };

int LoadCalls(const wchar_t *version, WxCalls_t *calls)
{
    if (wcscmp(version, SUPPORT_VERSION) != 0) {
        return -1;
    }

    memcpy_s(calls, sizeof(WxCalls_t), &wxCalls, sizeof(WxCalls_t));

    return 0;
}
