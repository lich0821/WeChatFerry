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
        // 共享辅助（多个发送函数复用；旧命名 TEXT_CALL1/CALL3、IMG_CALL1/CALL4）：
        constexpr uint32_t SEND_MGR_GETTER = 0x1197AC0;  // SendMessageMgr 单例 getter（读 dword_143682F4，空则 new(0xB0)+ctor sub_11773FD0）
        constexpr uint32_t CHATMSG_DTOR    = 0x1199010;  // ChatMsg::~ChatMsg，清理栈上临时 ChatMsg（对象恰好 0x2D8 字节，=Receive::CALL）

        // 发送文本（旧命名 TEXT_CALL2）：
        constexpr uint32_t SEND_MSG = 0x1783C10;  // SendMessageMgr::sendMsg（__fastcall，ecx=buffer/edx=wxid，6 栈参 msg/at/1/0/0/0）

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
        // HOOK = doAddMsg(SyncMgr, sub_117B8990) 尾部对“已收 ChatMsg”调用 ChatMsg::~ChatMsg 的 call 站点
        //        （0x117B93B5: `lea ecx,[ebp-418h]; call sub_11199010`，ecx=已解析 ChatMsg 基址）。
        //        因 CALL(=~ChatMsg) 有 50+ 调用点，不能直接 hook 其入口——改为 hook 入口后按“返回地址==HOOK+5”过滤，
        //        即仅当来自本站点时才 dispatch，等价于旧的裸 asm 定点 hook 且不引入内联汇编。
        constexpr uint32_t HOOK    = 0x17B93B5;  // 定点 call 站点（返回地址过滤用 HOOK+5）
        constexpr uint32_t CALL    = 0x1199010;  // ChatMsg::~ChatMsg 入口（detour 目标；前 5 字节 51 56 57 8B FE 可安全搬进 trampoline）
        // ---- ChatMsg 对象字段（相对已解析 ChatMsg 基址）----
        // 经 v3223/v31256 ctor+dtor 对比校验：低区（<0x48）不变；高区字段整体 +8（this+0x134 子对象内每个 std::string 后移 2 dword）。
        constexpr uint32_t MSG_ID  = 0x30;   // 不变
        constexpr uint32_t TYPE    = 0x38;   // 不变
        constexpr uint32_t IS_SELF = 0x3C;   // 不变
        constexpr uint32_t TS      = 0x44;   // 不变
        constexpr uint32_t ROOM_ID = 0x48;   // 不变（std::wstring）
        constexpr uint32_t CONTENT = 0x70;   // 不变（std::wstring）
        constexpr uint32_t WXID    = 0x188;  // 0x180 +8（子对象 std::string）
        constexpr uint32_t SIGN    = 0x19C;  // 0x194 +8
        constexpr uint32_t THUMB   = 0x1B0;  // 0x1A8 +8
        constexpr uint32_t EXTRA   = 0x1C4;  // 0x1BC +8
        constexpr uint32_t MSG_XML = 0x204;  // 0x1FC +8
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

    // ---- 已打开数据库管理器（进程内已解密句柄，扁平枚举）----
    // AccountStorageMgr::initStorage(sub_117254C0) 的 "init main storage" 循环：
    //   managerObj = *(g_WeChatWinDllAddr + INSTANCE)
    //   begin = *(managerObj + START); end = *(managerObj + END);  逐 4 字节遍历，*p = storage 对象指针
    //   storage 内：*(storage + SLOT) = 裸 sqlite3*（getHandle=vtable[2] 即 `return *(this+0x34)`，
    //   且 initStorage 把连接池解析出的句柄写回 storage+0x34，双重印证）；storage + NAME = std::wstring 库路径
    constexpr uint32_t INSTANCE = 0x4327610;  // AccountStorageMgr g_pInstance（dword_14327610，存管理器对象指针）
    constexpr uint32_t START    = 0x1430;     // managerObj → 主 storage 数组 begin 指针（+5168）
    constexpr uint32_t END      = 0x1434;     // managerObj → 主 storage 数组 end 指针（+5172）
    constexpr uint32_t SLOT     = 0x34;       // storage → 裸 sqlite3* 句柄
    constexpr uint32_t NAME     = 0x4C;       // storage → 库路径 std::wstring（_Bx 起始，容量位 +0x14）
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
    // 接收路径（#12 已解验，v3223/v31256 双会话）：
    //   CALL = SnsTimeLineMgr::OnSnsTimeLineSceneFinish 入口（朋友圈接收回调，__thiscall(this,a2,a3)，2 个调用者）；
    //   HOOK = OnProcessTimelineResp::<lambda_1> 内 `call OnSnsTimeLineSceneFinish(this,&container,0)` 定点，
    //          用返回地址 == HOOK+5 过滤（手法同 #11），a2 即 dispatch 的容器指针。
    constexpr uint32_t HOOK    = 0x1FDB1E5;
    constexpr uint32_t CALL    = 0x1FDB4D0;
    // CALL1/2/3（单例 getter / 首页 / 翻页刷新）仅 refresh_pyq(#28) 用，仍是 3.9.2.23 旧值，待 #28 迁移。
    constexpr uint32_t CALL1   = 0xC39680;
    constexpr uint32_t CALL2   = 0x14E2140;
    constexpr uint32_t CALL3   = 0x14E21E0;
    // 容器字段：主 feed 数组 begin/end 指针（容器 = a2）。
    constexpr uint32_t START   = 0x20;
    constexpr uint32_t END     = 0x24;
    // 单个 SnsObject（元素，STEP 字节）内字段（#12 重验：低区不变，XML 随元素增大位移）：
    constexpr uint32_t TS      = 0x2C;   // 不变（内层子对象 gap 内的 dword）
    constexpr uint32_t WXID    = 0x18;   // 不变（内层子对象首个 std::string）
    constexpr uint32_t CONTENT = 0x3C;   // 不变（内层子对象第 2 个 std::string）
    constexpr uint32_t XML     = 0x64C;  // 原 0x384；包装体高区第 3 个 string 块，元素增大后 dword225→dword403
    constexpr uint32_t STEP    = 0xE18;  // 原 0xB48；单个 SnsObject 大小 2888→3608
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
