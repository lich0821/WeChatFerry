#pragma once

#include <cstdint>

// x86 平台偏移地址定义
// 支持微信版本: 3.9.12.56

namespace Offsets
{

namespace Account
{
    constexpr uint32_t SERVICE = 0x4368508;  // 登录状态服务（AccountService 全局对象 RVA 0x4368308 + 0x200，登录后非零）
    constexpr uint32_t WXID    = 0x4368354;  // 微信ID（AccountService obj+0x4C，std::string，容量位 WXID+0x14）
    constexpr uint32_t NAME    = 0x4368460;  // 昵称（AccountService obj+0x158，第12个 std::string）
    constexpr uint32_t MOBILE  = 0x43683D0;  // 手机号（AccountService obj+0xC8，第6个 std::string）
    constexpr uint32_t HOME    = 0x434BD64;  // 数据目录（FileUtils p_WideCharStr，wchar_t* 指针，指向文档父目录）
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
    // ---- 裸 sqlite3 API（RVA，运行时 + g_WeChatWinDllAddr）----
    // 3.9.12.56 走进程内已解密句柄：从已开库管理器取裸 sqlite3*，直接调下列裸 sqlite 函数。
    constexpr uint32_t PREPARE_V2   = 0x2A97C50;  // sqlite3_prepare_v2(db, zSql, nByte, &ppStmt, &pzTail)
    constexpr uint32_t STEP         = 0x2A5E2E0;  // sqlite3_step(stmt) → 100=ROW/101=DONE
    constexpr uint32_t FINALIZE     = 0x2A5D330;  // sqlite3_finalize(stmt)
    constexpr uint32_t COLUMN_COUNT = 0x2A5E840;  // sqlite3_column_count(stmt)
    constexpr uint32_t COLUMN_NAME  = 0x2A5EEB0;  // sqlite3_column_name(stmt, iCol)
    constexpr uint32_t COLUMN_TYPE  = 0x2A5EDA0;  // sqlite3_column_type(stmt, iCol) 1I/2F/3T/4B/5NULL
    constexpr uint32_t COLUMN_BLOB  = 0x2A5E8E0;  // sqlite3_column_blob(stmt, iCol)
    constexpr uint32_t COLUMN_BYTES = 0x2A5E970;  // sqlite3_column_bytes(stmt, iCol)
    constexpr uint32_t COLUMN_TEXT  = 0x2A5EC60;  // sqlite3_column_text(stmt, iCol)

    // ---- 已打开数据库管理器（进程内已解密句柄）—— 待解析（#05 后续）----
    // AccountStorageMgr 单例（枚举根之一）；取句柄经虚函数 getHandle（vtable[2]，非固定偏移）。
    // TODO: 定位持有全部已打开库的“扁平总管理器”（每项含 库名 + 句柄），据此实现 库名→sqlite3* 枚举。
    constexpr uint32_t INSTANCE = 0x4327610;  // AccountStorageMgr g_pInstance（dword_14327610）
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
