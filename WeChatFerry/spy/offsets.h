#pragma once

#include <cstdint>

// x86 平台偏移地址定义
// 支持微信版本: 3.9.12.56

namespace Offsets
{

namespace Account
{
    constexpr uint32_t SERVICE = 0x4368508;  // 登录状态服务（登录后非零）
    constexpr uint32_t WXID    = 0x4368354;  // 微信ID（std::string，容量位 WXID+0x14）
    constexpr uint32_t NAME    = 0x4368460;  // 昵称（std::string）
    constexpr uint32_t MOBILE  = 0x43683D0;  // 手机号（std::string）
    constexpr uint32_t HOME    = 0x434BD64;  // 数据目录（wchar_t* 指向文档父目录）
    constexpr uint32_t LOGIN   = 0x0;        // 登录状态偏移（相对 SERVICE）
} // namespace Account

namespace Message
{
    namespace Send
    {
        // 共享辅助：
        constexpr uint32_t SEND_MGR_GETTER = 0x1197AC0;  // SendMessageMgr 单例 getter
        constexpr uint32_t CHATMSG_DTOR    = 0x1199010;  // ChatMsg::~ChatMsg（对象恰好 0x2D8 字节，=Receive::CALL）

        // 发送文本：
        constexpr uint32_t SEND_MSG = 0x1783C10;  // SendMessageMgr::sendMsg（__fastcall，ecx=buffer/edx=wxid，6 栈参 msg/at/1/0/0/0）

        // 发送图片（getter/dtor 与文本发送共用）：
        constexpr uint32_t SEND_IMAGE = 0x1783120;  // SendMessageMgr 图片提交叶子（__thiscall(mgr,buf,receiver,path,options)；options 布局见 message_sender.cpp）

        // 发送文件：走 AppMsgMgr::sendFile（getter=RichText::GETTER、assign=RichText::ASSIGN、dtor=CHATMSG_DTOR 复用）。
        // __usercall 建模为 __thiscall(ecx=manager，其余全部栈参)；receiver/path 及三个空串会被 mm_free，须用 ASSIGN 建拥有副本。
        constexpr uint32_t SEND_FILE = 0x1621EF0;  // AppMsgMgr::sendFile 提交叶子

        // send_xml 发原始 appmsg XML（type 为 appmsg 子类型，如 0x21 小程序）。__usercall 建模为 __fastcall(ecx=ChatMsg, edx=from)，
        // 原始 XML 走 a4→content、type→a7、flag=1 跳过内部 <msgsource>；各 WxString 仅被读取深拷贝进 ChatMsg（用非拥有视图即可）。
        constexpr uint32_t SEND_APPMSG  = 0x1621280;  // appmsg 发送核心（多入口共用）
        constexpr uint32_t CHATMSG_CTOR = 0x1A0ED0;   // ChatMsg 默认构造器（__thiscall(this)→this）

        // send_emotion 发本地自定义表情。SEND_CUSTOM_EMOTION 为 __thiscall(ecx=manager, retn 0x5C 被调清栈)；
        // path/wxid 及两个空串会被 mm_free，须用 ASSIGN 建拥有副本。buffer 为 0x1C 置零小结构，非 ChatMsg、无 dtor。
        constexpr uint32_t EMO_MGR_GETTER      = 0x1208250;  // CustomSmileyMgr 单例 getter（返回 &对象本体）
        constexpr uint32_t SEND_CUSTOM_EMOTION = 0x6C10C0;   // CustomSmileyMgr::sendCustomEmotion 发送叶子
    } // namespace Send

    namespace Receive
    {
        // 收消息定点：hook ~ChatMsg 入口后按“返回地址==HOOK+5”过滤，仅当来自 doAddMsg 尾部那条 call 时才 dispatch
        constexpr uint32_t HOOK    = 0x17B93B5;  // 定点 call 站点（返回地址过滤用 HOOK+5）
        constexpr uint32_t CALL    = 0x1199010;  // ChatMsg::~ChatMsg 入口（detour 目标；前 5 字节可安全搬进 trampoline）
        // ---- ChatMsg 对象字段（相对已解析 ChatMsg 基址）----
        constexpr uint32_t MSG_ID  = 0x30;
        constexpr uint32_t TYPE    = 0x38;
        constexpr uint32_t IS_SELF = 0x3C;
        constexpr uint32_t TS      = 0x44;
        constexpr uint32_t ROOM_ID = 0x48;   // std::wstring
        constexpr uint32_t CONTENT = 0x70;   // std::wstring
        constexpr uint32_t WXID    = 0x188;  // std::string
        constexpr uint32_t SIGN    = 0x19C;
        constexpr uint32_t THUMB   = 0x1B0;
        constexpr uint32_t EXTRA   = 0x1C4;
        constexpr uint32_t MSG_XML = 0x204;
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
    // managerObj = *(base + INSTANCE)；[managerObj+START, managerObj+END) 逐 4 字节遍历得 storage 对象指针；
    // storage+SLOT = 裸 sqlite3*，storage+NAME = 库路径 std::wstring。
    constexpr uint32_t INSTANCE = 0x4327610;  // AccountStorageMgr 单例（存管理器对象指针）
    constexpr uint32_t START    = 0x1430;     // managerObj → 主 storage 数组 begin 指针
    constexpr uint32_t END      = 0x1434;     // managerObj → 主 storage 数组 end 指针
    constexpr uint32_t SLOT     = 0x34;       // storage → 裸 sqlite3* 句柄
    constexpr uint32_t NAME     = 0x4C;       // storage → 库路径 std::wstring（容量位 +0x14）

    // ---- MSG / MediaMSG 多库（MultiDBMsgMgr，独立于上面的主 storage 数组）----
    // 消息与语音库由 MultiDBMsgMgr 单例的环形 item 指针数组管理（惰性构造，未登录时 mgr 为 0）：
    //   逻辑索引 i 的 item = *(base + 4 * ((i + head) & (cap - 1)))，cap 为 2 的幂。
    constexpr uint32_t MSG_MGR         = 0x436A8CC;  // MultiDBMsgMgr 单例对象指针
    constexpr uint32_t MSG_RING_BASE   = 0x2C;       // mgr → 环形 item 指针数组基址
    constexpr uint32_t MSG_RING_CAP    = 0x30;       // mgr → 环形容量（2 的幂）
    constexpr uint32_t MSG_RING_HEAD   = 0x34;       // mgr → 环形头索引
    constexpr uint32_t MSG_RING_COUNT  = 0x38;       // mgr → item 数量
    constexpr uint32_t MSG_ITEM_NAME   = 0x00;       // item → MSGn.db 名 WxString（wptr@0）
    constexpr uint32_t MSG_ITEM_HANDLE = 0x60;       // item → MSG 裸 sqlite3*
    constexpr uint32_t MSG_ITEM_MEDIA  = 0x14;       // item → MediaMSG 存储包装（StorageBase 派生）
    constexpr uint32_t MEDIA_NAME      = 0x4C;       // 媒体包装 → MediaMSGn.db 名 WxString（wptr@0）
    constexpr uint32_t MEDIA_HANDLE    = 0x38;       // 媒体包装 → 裸 sqlite3*

    // ---- 生库密钥（SQLCipher codec 链，供离线解密）----
    // 同一账号所有库共用同一 32 字节生密钥，从任一已开库句柄沿下链即可读出（无需 Hook）：
    //   db → +DB_BTREE → +BTREE_SHARED → +SHARED_PAGER → +PAGER_CODEC → +CODEC_READCTX=cipher_ctx
    //   → 密钥指针 +CIPHER_PASS、长度 +CIPHER_PASSSZ
    // 注：均为结构体字段偏移（非 RVA），使用时不加 g_WeChatWinDllAddr；仅登录并打开过库后才有效。
    constexpr uint32_t DB_BTREE      = 0x14;  // sqlite3* → aDb[0].pBt
    constexpr uint32_t BTREE_SHARED  = 0x04;  // Btree → BtShared
    constexpr uint32_t SHARED_PAGER  = 0x00;  // BtShared → Pager（首字段）
    constexpr uint32_t PAGER_CODEC   = 0xDC;  // Pager → codec（pCodec）
    constexpr uint32_t CODEC_READCTX = 0x54;  // codec → read cipher_ctx
    constexpr uint32_t CIPHER_PASS   = 0x10;  // cipher_ctx → 生密钥指针（pass）
    constexpr uint32_t CIPHER_PASSSZ = 0x04;  // cipher_ctx → 生密钥字节数（pass_sz）
} // namespace Database

namespace Friend
{
    // 通过好友申请：走 AddFriendHelper。VerifyOK 为纯 __thiscall(ecx=this, retn 0x30 被调清栈)；
    // v3(加密 username)仅被读取用非拥有视图，v4(ticket)会被 mm_free 须用 ASSIGN 建拥有副本。
    constexpr uint32_t ACCEPT_CTOR = 0x4CBD20;  // AddFriendHelper 构造器
    constexpr uint32_t VERIFY_OK   = 0x4CCE70;  // AddFriendHelper::VerifyOK
    constexpr uint32_t ACCEPT_DTOR = 0x4CBE40;  // AddFriendHelper 析构器
} // namespace Friend

namespace Chatroom
{
    // ChatRoomMgr 单例 getter，add/del/invite 三条链共用；roomid 均会被 mm_free，须用 ASSIGN 建拥有副本。
    constexpr uint32_t MGR_GETTER = 0x11C43A0;
    constexpr uint32_t ADD_MEMBER = 0x167F680;  // ChatRoomMgr::doAddMemberToChatRoom（__thiscall, retn 0x20）
    constexpr uint32_t DEL_MEMBER = 0x167FBD0;  // ChatRoomMgr::doDelMemberFromChatRoom（__thiscall, retn 0x18）
    // NetSceneInviteChatRoomMember 构建器 + 内部 doScene 发送（__stdcall(members, roomid 按值, 上下文 shared_ptr 按值)）；
    // 内部按 roomid 是否以 "@im.chatroom" 结尾分流普通/OpenIM，上下文 shared_ptr 传 {0,0}。
    constexpr uint32_t INVITE_MEMBER = 0x167F280;
} // namespace Chatroom

namespace Transfer
{
    // 领取转账走 WCPayInfo（带虚表）+ TenPayTransfer 受理入口。RECV_TRANSFER __usercall 建模为 __fastcall(ecx=payInfo, edx=wxid)；
    // wxid 只读用非拥有视图，transactionid/transferid 用 ASSIGN 建拥有副本（由 PAY_INFO_DTOR mm_free）。
    constexpr uint32_t PAY_INFO_CTOR  = 0x120AF00;  // WCPayInfo 默认构造器
    constexpr uint32_t RECV_TRANSFER  = 0x120DE350; // TenPayTransfer 受理入口（confirm=1 接受/=0 拒绝）
    constexpr uint32_t PAY_INFO_DTOR  = 0x111D4570; // WCPayInfo 析构器

    // WCPayInfo 字段偏移。
    constexpr uint32_t F_PAYSUBTYPE     = 0x04;  // 支付子类型 int
    constexpr uint32_t F_TRANSACTION_ID = 0x1C;  // 交易号 WxString
    constexpr uint32_t F_TRANSFER_ID    = 0x38;  // 转账单号 WxString
    constexpr uint32_t F_EFFECTIVE_DATE = 0x4C;  // 生效标志 int
} // namespace Transfer

namespace Moments
{
    // 接收路径：hook OnSnsTimeLineSceneFinish 入口后按返回地址==HOOK+5 过滤，此时 a2 即容器指针
    constexpr uint32_t HOOK    = 0x1FDB1E5;
    constexpr uint32_t CALL    = 0x1FDB4D0;  // SnsTimeLineMgr::OnSnsTimeLineSceneFinish 入口（2 个调用者）
    // 刷新路径（refresh_pyq）：两个 Scene 构建器内部自建 NetScene 经 doScene 直接发送
    constexpr uint32_t MGR_GETTER     = 0x1F77D90;  // SnsTimeLineMgr 单例 getter
    constexpr uint32_t GET_FIRST_PAGE = 0x1FDE8A0;  // TryGetFirstPageScene（__thiscall(manager, forward), retn 4）
    constexpr uint32_t GET_NEXT_PAGE  = 0x1FDED90;  // GetNextPageScene（__thiscall(manager, id_low, id_high), retn 8）
    // 容器字段：主 feed 数组 begin/end 指针（容器 = a2）
    constexpr uint32_t START   = 0x20;
    constexpr uint32_t END     = 0x24;
    // 单个 SnsObject（元素，STEP 字节）内字段：
    constexpr uint32_t TS      = 0x2C;
    constexpr uint32_t WXID    = 0x18;   // 内层子对象首个 std::string
    constexpr uint32_t CONTENT = 0x3C;   // 内层子对象第 2 个 std::string
    constexpr uint32_t XML     = 0x64C;
    constexpr uint32_t STEP    = 0xE18;  // 单个 SnsObject 大小
} // namespace Moments

namespace Attachment
{
    // 下载附件走 PreDownLoadMgr（ChatMsg 缓冲复用 Send::CHATMSG_CTOR/DTOR，路径拷贝复用 RichText::ASSIGN）。
    // LOAD_MSG（ChatMgr::GetMgrByPrefixLocalId）__usercall 建模为 __thiscall，与 forward 共用；
    // PUSH_TASK 纯 __thiscall(retn 0x10)，调用形如 push_attach_task(manager, buffer, 0, 1, 0)。
    constexpr uint32_t WARMUP     = 0x11C7290;  // ChatMgr 懒初始化
    constexpr uint32_t LOAD_MSG   = 0x166FE00;  // ChatMgr::GetMgrByPrefixLocalId（按 localId/dbIdx 加载消息进 ChatMsg）
    constexpr uint32_t MGR_GETTER = 0x1240DE0;  // PreDownLoadMgr 单例 getter
    constexpr uint32_t PUSH_TASK  = 0x12DA8A0;  // PreDownLoadMgr::push_attach_task

    // ChatMsg 内下载目标字段（type 读 Message::Receive::TYPE）。路径字段会被 ~ChatMsg mm_free，须用 ASSIGN 建拥有副本。
    constexpr uint32_t F_THUMB_PATH  = 0x1A4;  // 缩略图落盘路径 WxString
    constexpr uint32_t F_SAVE_PATH   = 0x1B8;  // 附件落盘路径 WxString
    constexpr uint32_t F_INITED_FLAG = 0x2AC;  // "子对象已初始化"标志（置 1 跳过重复 init）
} // namespace Attachment

namespace Revoke
{
    // 撤回消息走 ChatRevokeMgr::revokeMsg（ChatMsg 缓冲复用 Send::CHATMSG_CTOR/DTOR，加载消息复用 Attachment::LOAD_MSG）。
    // REVOKE_MSG 纯 __thiscall(ecx=manager, chat_msg, retn 4)。
    constexpr uint32_t MGR_GETTER = 0x1245280;  // ChatRevokeMgr 单例 getter（返回 &对象本体）
    constexpr uint32_t REVOKE_MSG = 0x12469F0;  // ChatRevokeMgr::revokeMsg
} // namespace Revoke

namespace RichText
{
    // 发送消息卡片：业务对象 MMReaderItem + AppMsgMgr 单例。
    // 编排：CTOR(buff) → 用 ASSIGN 逐字段深拷贝 → GETTER()=manager → SEND(manager, receiver 按值, buff) → DTOR(buff)。
    // receiver 会被 SEND mm_free，须用 ASSIGN 建 WeChat 拥有副本。
    constexpr uint32_t CTOR   = 0x11A0220;  // MMReaderItem::MMReaderItem（__thiscall(this)）
    constexpr uint32_t GETTER = 0x119C990;  // AppMsgMgr 单例 getter
    constexpr uint32_t ASSIGN = 0x19DAF20;  // WxString::assign(src,len)（__thiscall, mm_realloc 深拷贝）
    constexpr uint32_t SEND   = 0x1628DC0;  // AppMsgMgr::sendAppMsg（__thiscall(manager, WxString receiver 按值, MMReaderItem* buff)）
    constexpr uint32_t DTOR   = 0x119F530;  // MMReaderItem::~MMReaderItem 完整析构

    // MMReaderItem 内 WxString 成员偏移（=该成员 wptr 的字节偏移）。
    constexpr uint32_t F_TITLE    = 0x4;    // 标题
    constexpr uint32_t F_URL      = 0x2C;   // 链接
    constexpr uint32_t F_THUMBURL = 0x6C;   // 缩略图 URL
    constexpr uint32_t F_DIGEST   = 0x94;   // 摘要
    constexpr uint32_t F_ACCOUNT  = 0x1B4;  // 源用户名
    constexpr uint32_t F_NAME     = 0x1C8;  // 源显示名
    constexpr uint32_t OBJ_SIZE   = 0x280;  // MMReaderItem 栈缓冲大小
} // namespace RichText

namespace Pat
{
    // 拍一拍走 PatMgr。SEND_PAT（PatMgr::SendPatMsg）__usercall 建模为 __fastcall(ecx=roomid, edx=wxid)，
    // 栈参 manager/0/0，返回 al 非零即成功。
    constexpr uint32_t MGR_GETTER = 0x124A670;
    constexpr uint32_t SEND_PAT   = 0x1EB5D50;
} // namespace Pat

namespace OCR
{
    // 图片 OCR 走 OCRManager。RUN_OCR（DoOCRTask）__usercall 建模为 __thiscall(ecx=manager, 5 栈参)，返回 task_id；
    // 异步：仅缓存命中时同步回填 result_list 并置 found_flag=1、返回 0（结点文本 WxString 在 node+0x14）。
    constexpr uint32_t MGR_GETTER  = 0x12B5F50;
    constexpr uint32_t RUN_OCR     = 0x1E7A2E0;
    constexpr uint32_t RESULT_NEW  = 0x3234B97;  // 全局 operator new（分配链表哨兵结点，与 RESULT_DTOR 配对）
    constexpr uint32_t RESULT_DTOR = 0x12B6140;  // OCR 结果链表析构器
} // namespace OCR

namespace Forward
{
    // 转发消息走 SendMessageMgr::forwordMsg。__usercall 建模为 __fastcall(ecx=scene, edx=msgPtr)，
    // 栈参 receiver 按值 + localId + dbIdx；msgPtr=0 走加载路径、scene=5 仅统计元数据；
    // receiver 会被末尾 mm_free，须用 RichText::ASSIGN 建拥有副本。
    constexpr uint32_t FORWARD_MSG = 0x1783230;  // SendMessageMgr::forwordMsg
} // namespace Forward

namespace QRCode
{
    // 刷新登录二维码走 QRCodeLoginMgr。getQRCodeImage 经 doScene 异步获取，完成后回填 uuid；
    // URL 是 MSVC std::string（size@+0x10、cap@+0x14，短串走 SSO 地址即缓冲区、长串 +0 处为堆指针）。
    constexpr uint32_t MGR_GETTER = 0x1589600;  // QRCodeLoginMgr 单例 getter
    constexpr uint32_t GET_QRCODE = 0x17725A0;  // QRCodeLoginMgr::getQRCodeImage
    constexpr uint32_t URL        = 0x436C398;  // 登录 uuid std::string（管理器 +8）
} // namespace QRCode

} // namespace Offsets
