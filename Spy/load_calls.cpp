#include <iostream>
#include <map>

#include "load_calls.h"

#define SUPPORT_VERSION L"3.3.0.115"
WxCalls_t wxCalls = { 0x1DDF60C,                           // Login Status
                      { 0x1DDF4BC, 0x1DDF534, 0x1DDF568 }, // User Info: wxid, nickname, mobile
                      0x3E3B80,                            // Send Message
                      /* Receive Message:
                            Hook,   call,   type, self,   id, msgXml, roomId, wxId, content */
                      { 0x3C0D70, 0x3C0FA0, 0x38, 0x3C, 0x184, 0x1D8, 0x48, 0x170, 0x70 } };

int LoadCalls(const wchar_t *version, WxCalls_t *calls)
{
    if (wcscmp(version, SUPPORT_VERSION) != 0) {
        return -1;
    }

    memcpy_s(calls, sizeof(WxCalls_t), &wxCalls, sizeof(WxCalls_t));

    return 0;
}
