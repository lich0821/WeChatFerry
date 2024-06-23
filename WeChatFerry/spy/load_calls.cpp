#include <iostream>
#include <map>

#include "load_calls.h"

#define SUPPORT_VERSION L"3.9.10.27"

WxCalls_t wxCalls = {
    //0x5AB8A2C,                                      // Login Status
    //{ 0x5AB7FB8, 0x5AB8098, 0x5AB7FD8, 0x5A7E190 }, // User Info: wxid, nickname, mobile, home
    //{ 0x1C1E690, 0x238DDD0, 0x1C1FF10 },            // Send Text Message
    ///* Receive Message:
    //    Hook,  call, msgId, type, isSelf, ts, roomId, content, wxid, sign, thumb, extra, msgXml */
    //{ 0x00, 0x2205510, 0x30, 0x38, 0x3C, 0x44, 0x48, 0x88, 0x240, 0x260, 0x280, 0x2A0, 0x308 },
    //{ 0x1C28800, 0x1C1FF10, 0x1C1E690, 0x2383560 },       // Send Image Message
    //{ 0x1C28800, 0x1C1FF10, 0x1C23630, 0x21969E0 },       // Send File Message
    //{ 0xB8A70, 0x3ED5E0, 0x107F00, 0x3ED7B0, 0x2386FE4 }, // Send xml Message
    //{ 0x771980, 0x4777E0, 0x239E888 },                    // Send Emotion Message
    ///* Get Contacts:
    //    call1, call2, wxId, Code, Remark,Name, Gender, Country, Province, City*/
    //{ 0x75A4A0, 0xC089F0, 0x10, 0x24, 0x58, 0x6C, 0x0E, 0x00, 0x00, 0x00 },
    ///* Exec Sql:
    //      Exec,     base,   start,   end,   slot, name*/
    //{ 0x141BDF0, 0x2366934, 0x1428, 0x142C, 0x3C, 0x50 },
    //{ 0xA17D50, 0xF59E40, 0xA18BD0, 0xA17E70 }, // Accept New Friend application
    //{ 0x78CF20, 0xF59E40, 0xBD1DC0 },           // Add chatroom members
    //{ 0x78CF20, 0xF59E40, 0xBD22A0 },           // Delete chatroom members
    //{ 0x7B2E60, 0x15E2C20, 0x79C250 },          // Receive transfer
    ///* Receive PYQ
    //    hook,    call,     call1,    call2,    call3,      start, end,  ts,  wxid, content, xml, step*/
    //{ 0x14F9E15, 0x14FA0A0, 0xC39680, 0x14E2140, 0x14E21E0, 0x20, 0x24, 0x2C, 0x18, 0x3C, 0x384, 0xB48 },
    ///*  call1,    call2,    call3,    call4,    call5,    call6*/
    //{ 0x76F010, 0x792700, 0xBC0370, 0x80F110, 0x82BB40, 0x756E30},
    ///*  call1,    call2,    call3,    call4,    call5*/
    //{0x76F010, 0x792700, 0xBC0370, 0xBB5F70, 0x756E30},
    //{0x1C27D50, 0x1C27120, 0x1C23630, 0x21A09C0},  // Send Rich Text Message
    ///*  call1,    call2,    call3 */
    //{0x931730, 0x1D58751, 0x1421940},
    ///*  call1,    call2,    call3,    call4,    call5,    call6,    call7,    call8*/
    //{0x78CB40, 0x7F99D0, 0x78CF20, 0x78CEF0, 0xF59E40, 0xBD1A00, 0x7FA980, 0x755060},
    ///*  call1,    call2,    call3 */
    //{0x80A800, 0x80F270, 0x13DA3E0},
    ///*  call1,   call2 */
    //{0xF59E40, 0xCE6730},
    ///*  call1,   call2,     url */
    //{0xAE9DB0, 0xCDA6F0, 0x3040DE8}
};

int LoadCalls(const wchar_t *version, WxCalls_t *calls)
{
    if (wcscmp(version, SUPPORT_VERSION) != 0) {
        return -1;
    }

    memcpy_s(calls, sizeof(WxCalls_t), &wxCalls, sizeof(WxCalls_t));

    return 0;
}
