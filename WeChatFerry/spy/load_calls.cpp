#include <iostream>
#include <map>

#include "load_calls.h"

#define SUPPORT_VERSION L"3.9.2.23"
WxCalls_t wxCalls = {
    0x2FFD638,                                      // Login Status
    { 0x2FFD484, 0x2FFD590, 0x2FFD500, 0x30238CC }, // User Info: wxid, nickname, mobile, home
    { 0x768140, 0xCE6C80, 0x756960 },               // Send Message
    /* Receive Message:
          Hook,  call, msgId, type, isSelf, ts, roomId, content, wxid, sign, thumb, extra, msgXml */
    { 0xD19A0B, 0x756960, 0x30, 0x38, 0x3C, 0x44, 0x48, 0x70, 0x180, 0x194, 0x1A8, 0x1BC, 0x1FC },
    { 0x768140, 0XF59E40, 0XCE6640, 0x756960 },           // Send Image Message
    { 0x76AE20, 0xF59E40, 0xB6D1F0, 0x756960 },           // Send File Message
    { 0xB8A70, 0x3ED5E0, 0x107F00, 0x3ED7B0, 0x2386FE4 }, // Send xml Message
    { 0x771980, 0x4777E0, 0x239E888 },                    // Send Emotion Message
    /* Get Contacts:
        call1, call2, wxId, Code, Remark,Name, Gender, Country, Province, City*/
    { 0x75A4A0, 0xC089F0, 0x10, 0x24, 0x58, 0x6C, 0x0E, 0x00, 0x00, 0x00 },
    /* Exec Sql:
          Exec,     base,   start,   end,   slot, name*/
    { 0x141BDF0, 0x2366934, 0x1428, 0x142C, 0x3C, 0x50 },
    { 0xA17D50, 0xF59E40, 0xA18BD0, 0xA17E70 }, // Accept New Friend application
    { 0x78CF20, 0xF59E40, 0xBD1DC0 },           // Add chatroom members
    { 0x78CF20, 0xF59E40, 0xBD22A0 },           // Delete chatroom members
    { 0x7B2E60, 0x15E2C20, 0x79C250 },          // Receive transfer
    /* Receive PYQ
        hook,    call,     call1,    call2,    call3,      start, end,  ts,  wxid, content, xml, step*/
    { 0x14F9E15, 0x14FA0A0, 0xC39680, 0x14E2140, 0x14E21E0, 0x20, 0x24, 0x2C, 0x18, 0x3C, 0x384, 0xB48 },
    /*  call1,    call2,    call3,    call4,    call5,    call6*/
    { 0x76F010, 0x792700, 0xBC0370, 0x80F110, 0x82BB40, 0x756E30},
    /*  call1,    call2,    call3,    call4,    call5*/
    {0x76F010, 0x792700, 0xBC0370, 0xBB5F70, 0x756E30},
    /*  call1,    call2,    call3,    call4,    call5*/
    {0x76E630, 0x76AE20, 0xF59E40, 0xB73000, 0x76E350},
    /*  call1,    call2,    call3 */
    {0x931730, 0x1D58751, 0x1421940},
    /*  call1,    call2,    call3,    call4,    call5,    call6,    call7,    call8*/
    {0x78CB40, 0x7F99D0, 0x78CF20, 0x78CEF0, 0xF59E40, 0xBD1A00, 0x7FA980, 0x755060},
    /*  call1,    call2,    call3 */
    {0x80A800, 0x80F270, 0x13DA3E0},
    /*  call1,   call2 */
    {0xF59E40, 0xCE6730}
};

int LoadCalls(const wchar_t *version, WxCalls_t *calls)
{
    if (wcscmp(version, SUPPORT_VERSION) != 0) {
        return -1;
    }

    memcpy_s(calls, sizeof(WxCalls_t), &wxCalls, sizeof(WxCalls_t));

    return 0;
}
