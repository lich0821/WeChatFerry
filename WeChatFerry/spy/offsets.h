#pragma once

#include <cstdint>

// x86 平台偏移地址定义
// 支持微信版本: 3.9.2.23

namespace Offsets
{

namespace Account
{
    constexpr uint32_t SERVICE = 0x2FFD638;  // 登录状态服务
    constexpr uint32_t WXID    = 0x2FFD484;  // 微信ID
    constexpr uint32_t NAME    = 0x2FFD590;  // 昵称
    constexpr uint32_t MOBILE  = 0x2FFD500;  // 手机号
    constexpr uint32_t HOME    = 0x30238CC;  // 数据目录
    constexpr uint32_t LOGIN   = 0x0;        // 登录状态偏移（相对于SERVICE）
} // namespace Account

namespace Message
{
    namespace Send
    {
        constexpr uint32_t TEXT_CALL1 = 0x768140;
        constexpr uint32_t TEXT_CALL2 = 0xCE6C80;
        constexpr uint32_t TEXT_CALL3 = 0x756960;

        constexpr uint32_t IMG_CALL1 = 0x768140;
        constexpr uint32_t IMG_CALL2 = 0xF59E40;
        constexpr uint32_t IMG_CALL3 = 0xCE6640;
        constexpr uint32_t IMG_CALL4 = 0x756960;

        constexpr uint32_t FILE_CALL1 = 0x76AE20;
        constexpr uint32_t FILE_CALL2 = 0xF59E40;
        constexpr uint32_t FILE_CALL3 = 0xB6D1F0;
        constexpr uint32_t FILE_CALL4 = 0x756960;

        constexpr uint32_t XML_CALL1 = 0xB8A70;
        constexpr uint32_t XML_CALL2 = 0x3ED5E0;
        constexpr uint32_t XML_CALL3 = 0x107F00;
        constexpr uint32_t XML_CALL4 = 0x3ED7B0;
        constexpr uint32_t XML_PARAM = 0x2386FE4;

        constexpr uint32_t EMO_CALL1 = 0x771980;
        constexpr uint32_t EMO_CALL2 = 0x4777E0;
        constexpr uint32_t EMO_CALL3 = 0x239E888;
    } // namespace Send

    namespace Receive
    {
        constexpr uint32_t HOOK    = 0xD19A0B;
        constexpr uint32_t CALL    = 0x756960;
        constexpr uint32_t MSG_ID  = 0x30;
        constexpr uint32_t TYPE    = 0x38;
        constexpr uint32_t IS_SELF = 0x3C;
        constexpr uint32_t TS      = 0x44;
        constexpr uint32_t ROOM_ID = 0x48;
        constexpr uint32_t CONTENT = 0x70;
        constexpr uint32_t WXID    = 0x180;
        constexpr uint32_t SIGN    = 0x194;
        constexpr uint32_t THUMB   = 0x1A8;
        constexpr uint32_t EXTRA   = 0x1BC;
        constexpr uint32_t MSG_XML = 0x1FC;
    } // namespace Receive
} // namespace Message

namespace Contact
{
    constexpr uint32_t BASE     = 0x75A4A0;
    constexpr uint32_t HEAD     = 0xC089F0;
    constexpr uint32_t WXID     = 0x10;
    constexpr uint32_t CODE     = 0x24;
    constexpr uint32_t REMARK   = 0x58;
    constexpr uint32_t NAME     = 0x6C;
    constexpr uint32_t GENDER   = 0x0E;
    constexpr uint32_t COUNTRY  = 0x00;
    constexpr uint32_t PROVINCE = 0x00;
    constexpr uint32_t CITY     = 0x00;
} // namespace Contact

namespace Database
{
    constexpr uint32_t EXEC  = 0x141BDF0;
    constexpr uint32_t BASE  = 0x2366934;
    constexpr uint32_t START = 0x1428;
    constexpr uint32_t END   = 0x142C;
    constexpr uint32_t SLOT  = 0x3C;
    constexpr uint32_t NAME  = 0x50;
} // namespace Database

namespace Friend
{
    constexpr uint32_t ACCEPT_CALL1 = 0xA17D50;
    constexpr uint32_t ACCEPT_CALL2 = 0xF59E40;
    constexpr uint32_t ACCEPT_CALL3 = 0xA18BD0;
    constexpr uint32_t ACCEPT_CALL4 = 0xA17E70;
} // namespace Friend

namespace Chatroom
{
    constexpr uint32_t ADD_CALL1 = 0x78CF20;
    constexpr uint32_t ADD_CALL2 = 0xF59E40;
    constexpr uint32_t ADD_CALL3 = 0xBD1DC0;

    constexpr uint32_t DEL_CALL1 = 0x78CF20;
    constexpr uint32_t DEL_CALL2 = 0xF59E40;
    constexpr uint32_t DEL_CALL3 = 0xBD22A0;

    constexpr uint32_t INV_CALL1 = 0x78CB40;
    constexpr uint32_t INV_CALL2 = 0x7F99D0;
    constexpr uint32_t INV_CALL3 = 0x78CF20;
    constexpr uint32_t INV_CALL4 = 0x78CEF0;
    constexpr uint32_t INV_CALL5 = 0xF59E40;
    constexpr uint32_t INV_CALL6 = 0xBD1A00;
    constexpr uint32_t INV_CALL7 = 0x7FA980;
    constexpr uint32_t INV_CALL8 = 0x755060;
} // namespace Chatroom

namespace Transfer
{
    constexpr uint32_t CALL1 = 0x7B2E60;
    constexpr uint32_t CALL2 = 0x15E2C20;
    constexpr uint32_t CALL3 = 0x79C250;
} // namespace Transfer

namespace Moments
{
    constexpr uint32_t HOOK    = 0x14F9E15;
    constexpr uint32_t CALL    = 0x14FA0A0;
    constexpr uint32_t CALL1   = 0xC39680;
    constexpr uint32_t CALL2   = 0x14E2140;
    constexpr uint32_t CALL3   = 0x14E21E0;
    constexpr uint32_t START   = 0x20;
    constexpr uint32_t END     = 0x24;
    constexpr uint32_t TS      = 0x2C;
    constexpr uint32_t WXID    = 0x18;
    constexpr uint32_t CONTENT = 0x3C;
    constexpr uint32_t XML     = 0x384;
    constexpr uint32_t STEP    = 0xB48;
} // namespace Moments

namespace Attachment
{
    constexpr uint32_t DL_CALL1 = 0x76F010;
    constexpr uint32_t DL_CALL2 = 0x792700;
    constexpr uint32_t DL_CALL3 = 0xBC0370;
    constexpr uint32_t DL_CALL4 = 0x80F110;
    constexpr uint32_t DL_CALL5 = 0x82BB40;
    constexpr uint32_t DL_CALL6 = 0x756E30;
} // namespace Attachment

namespace Revoke
{
    constexpr uint32_t CALL1 = 0x76F010;
    constexpr uint32_t CALL2 = 0x792700;
    constexpr uint32_t CALL3 = 0xBC0370;
    constexpr uint32_t CALL4 = 0xBB5F70;
    constexpr uint32_t CALL5 = 0x756E30;
} // namespace Revoke

namespace RichText
{
    constexpr uint32_t CALL1 = 0x76E630;
    constexpr uint32_t CALL2 = 0x76AE20;
    constexpr uint32_t CALL3 = 0xF59E40;
    constexpr uint32_t CALL4 = 0xB73000;
    constexpr uint32_t CALL5 = 0x76E350;
} // namespace RichText

namespace Pat
{
    constexpr uint32_t CALL1 = 0x931730;
    constexpr uint32_t CALL2 = 0x1D58751;
    constexpr uint32_t CALL3 = 0x1421940;
} // namespace Pat

namespace OCR
{
    constexpr uint32_t CALL1 = 0x80A800;
    constexpr uint32_t CALL2 = 0x80F270;
    constexpr uint32_t CALL3 = 0x13DA3E0;
} // namespace OCR

namespace Forward
{
    constexpr uint32_t CALL1 = 0xF59E40;
    constexpr uint32_t CALL2 = 0xCE6730;
} // namespace Forward

namespace QRCode
{
    constexpr uint32_t CALL1 = 0xAE9DB0;
    constexpr uint32_t CALL2 = 0xCDA6F0;
    constexpr uint32_t URL   = 0x3040DE8;
} // namespace QRCode

} // namespace Offsets
