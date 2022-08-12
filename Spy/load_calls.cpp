#include <iostream>
#include <map>

#include "load_calls.h"

#define SUPPORT_VERSION L"3.7.0.30"
WxCalls_t wxCalls = { 0x2366538,                           // Login Status
                      { 0x236607C, 0x23660F4, 0x2366128 }, // User Info: wxid, nickname, mobile
                      0x521D30,                            // Send Message
                      /* Receive Message:
                            Hook,   call,   type, self,   id, msgXml, roomId, wxId, content */
                      { 0x550F4C, 0xA96350, 0x38, 0x3C, 0x184, 0x1EC, 0x48, 0x170, 0x70 },
                      { 0xBD780, 0x771980, 0x521640 }, // Send Image Message
                      /* Get Contacts:
                            Base,  head, wxId, Code, Name, Gender, Country, Province, City*/
                      { 0x23638F4, 0x4C, 0x30, 0x44, 0x8C, 0x184, 0x1D0, 0x1E4, 0x1F8 },
                      /* Exec Sql:
                            Exec,     base,   start,   end,   slot, name*/
                      { 0x141A4D0, 0x2363934, 0x1428, 0x142C, 0x3C, 0x50 } };

int LoadCalls(const wchar_t *version, WxCalls_t *calls)
{
    if (wcscmp(version, SUPPORT_VERSION) != 0) {
        return -1;
    }

    memcpy_s(calls, sizeof(WxCalls_t), &wxCalls, sizeof(WxCalls_t));

    return 0;
}
