#include <iostream>
#include <map>

#include "load_calls.h"

#define SUPPORT_VERSION L"3.7.0.29"
WxCalls_t wxCalls = {
    0x23631D0,                           // Login Status
    { 0x236307C, 0x23630F4, 0x2363128 }, // User Info: wxid, nickname, mobile
    0x3E3B80,                            // Send Message
    /* Receive Message:
          Hook,   call,   type, self,   id, msgXml, roomId, wxId, content */
    { 0x550F4C, 0xA94A50, 0x38, 0x3C, 0x184, 0x1EC, 0x48, 0x170, 0x70 },
    { 0x5CCB50, 0x6F5C0, 0x3E3490 } // Send Image Message
};

int LoadCalls(const wchar_t* version, WxCalls_t* calls)
{
    if (wcscmp(version, SUPPORT_VERSION) != 0) {
        return -1;
    }

    memcpy_s(calls, sizeof(WxCalls_t), &wxCalls, sizeof(WxCalls_t));

    return 0;
}
