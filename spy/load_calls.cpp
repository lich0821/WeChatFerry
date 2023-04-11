#include <iostream>
#include <map>

#include "load_calls.h"

#define SUPPORT_VERSION L"3.7.0.30"
WxCalls_t wxCalls = {
    0x2366538,                                      // Login Status
    { 0x236607C, 0x23660F4, 0x2366128, 0x2386F7C }, // User Info: wxid, nickname, mobile, home
    0x521D30,                                       // Send Message
    /* Receive Message:
          Hook,   call,   type, self,   id, msgXml, roomId, wxId, content, thumb, extra */
    { 0x550F4C, 0xA96350, 0x38, 0x3C, 0x184, 0x1EC, 0x48, 0x170, 0x70, 0x198, 0x1AC },
    { 0xBD780, 0x771980, 0x521640 },                      // Send Image Message
    { 0xC3B70, 0x771980, 0x3ED8C0 },                      // Send File Message
    { 0xB8A70, 0x3ED5E0, 0x107F00, 0x3ED7B0, 0x2386FE4 }, // Send xml Message
    { 0x771980, 0x4777E0, 0x239E888 },                    // Send Emotion Message
    /* Get Contacts:
          Base,  head, wxId, Code, Name, Gender, Country, Province, City*/
    { 0x23668F4, 0x4C, 0x30, 0x44, 0x8C, 0x184, 0x1D0, 0x1E4, 0x1F8 },
    /* Exec Sql:
          Exec,     base,   start,   end,   slot, name*/
    { 0x141BDF0, 0x2366934, 0x1428, 0x142C, 0x3C, 0x50 },
    { 0x771980, 0x2AE8D0, 0x1EE40E0 }, // Accept New Friend application
    { 0xE29F0, 0x771980, 0x43D8D0 }    // Add chatroom members
};

int LoadCalls(const wchar_t *version, WxCalls_t *calls)
{
    if (wcscmp(version, SUPPORT_VERSION) != 0) {
        return -1;
    }

    memcpy_s(calls, sizeof(WxCalls_t), &wxCalls, sizeof(WxCalls_t));

    return 0;
}
