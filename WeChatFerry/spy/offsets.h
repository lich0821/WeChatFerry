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
        // 共享辅助：
        constexpr uint32_t SEND_MGR_GETTER = 0x1197AC0;  // SendMessageMgr 单例 getter（读 dword_143682F4，空则 new(0xB0)+ctor sub_11773FD0）
        constexpr uint32_t CHATMSG_DTOR    = 0x1199010;  // ChatMsg::~ChatMsg，清理栈上临时 ChatMsg（对象恰好 0x2D8 字节，=Receive::CALL）

        // 发送文本：
        constexpr uint32_t SEND_MSG = 0x1783C10;  // SendMessageMgr::sendMsg（__fastcall，ecx=buffer/edx=wxid，6 栈参 msg/at/1/0/0/0）

        // 发送图片：getter=SEND_MGR_GETTER、dtor=CHATMSG_DTOR 与文本发送共用
        //   不再需要（WxString 直接由 C++ 构造）。校验依据：ChatViewModel::reSendMsg(sub_113CE240) 的 case 3
        //   与多个真实调用者均为 getter()→SEND_IMAGE(mgr,buf,receiver,path,options)→~ChatMsg(buf)。
        constexpr uint32_t SEND_IMAGE = 0x1783120;  // SendMessageMgr 图片提交叶子 sub_11783120（旧 IMG_CALL3=0xCE6640）
                                                    //   __thiscall(mgr, buf, receiver, path, options*)；options 布局见 message_sender.cpp

        // 发送文件：走 AppMsgMgr::sendFile。
        //   getter=RichText::GETTER(0x119C990, AppMsgMgr 单例)、assign=RichText::ASSIGN(0x19DAF20, WxString::assign)、
        //   dtor=CHATMSG_DTOR(0x1199010, ChatMsg::~ChatMsg) 三者与卡片/文本发送共用；本命名空间仅新增文件提交叶子 SEND_FILE。
        // 定位：
        //   sub_11621EF0（结构 1:1：串 "copy err,src:%s"/AppMsgMgr.cpp:1143、登录检查、末尾对各 WxString mm_free）。
        // ABI：__usercall 对齐栈 prologue(push ebx;mov ebx,esp;and esp,-8) → caller-clean，以 __thiscall 建模
        //   （ecx=manager，其余 32 个全部栈参），栈失衡由 send_file() 自身帧指针 epilogue 纠正。
        //   参数（据 ChatViewModel::reSendMsg(sub_113CE240) case 0x31 精确核对，a3..a34）：
        //     buffer(输出 ChatMsg) + receiver(WxString) + path(WxString) + 1 + 空 WxString + 0 + 空 WxString + 0 + 0 + 空 WxString + 0 + 0。
        //   receiver/path 及三个空 WxString 全部被 sendFile mm_free，故须用 ASSIGN 建 WeChat 拥有副本、勿传 std::wstring 别名。
        constexpr uint32_t SEND_FILE = 0x1621EF0;  // AppMsgMgr::sendFile 提交叶子（旧 FILE_CALL3=0xB6D1F0）

        // send_xml 发原始 appmsg XML（type 为 appmsg 子类型，如 0x21 小程序）。
        // 定位：
        //   new_chat_msg×2 → xml_buf_sign(buf2,array,1)=sign → send_xml(buf1,from,to,body,thumb,buf3,type,4,sign,buf2) → free×2。
        //   x86 3.9.12.56 已把该 10 参单体 + sign 生成合并进一个共享 appmsg 发送核心 sub_11621280（sign 由其内部 sub_11672DA0 生成、无需外部算）。
        //   sub_11621280 被 sendFile/forwardAppMsg/sendQuoteMsg/安全提示 appmsg 等 10 个入口共用；照抄最简调用者
        //   sub_1162A3B0（发固定安全中心 appmsg）的调用点反汇编逐参确认。CHATMSG_CTOR/CHATMSG_DTOR/RichText::ASSIGN 复用。
        // ABI：__usercall caller-clean（调用点 `add esp,0x20`=8 栈参），
        //   ecx=a1、edx=a2，以 __fastcall 建模、栈失衡由 send_xml() 自身帧指针 epilogue 纠正（同 send_pat/forward/send_file）。
        //   参数：a1(ecx)=ChatMsg*（须先 CHATMSG_CTOR 构造）、a2(edx)=from 自己 wxid WxString*(→ChatMsg+380，源自 AccountServiceMgr，
        //   与 x64 参考 arg2=from 吻合)、a3=收件人 WxString*(→+72 talker)、a4=原始 XML WxString*(→+112 content)、a5=空 WxString*、
        //   a6=封面图 path WxString*(仅 subType==6 时用)、a7=type(appmsg 子类型→+244)、a8=flag(≠1 时内部建 <msgsource>，取 1 跳过)、
        //   a9=空 WxString*、a10=0。入口守卫 *(a2+4)/*(a3+4)/*(a4+4) 均须>0（from/receiver/xml 非空）。
        // 所有权：0x11621280 仅读取各 WxString 并深拷贝进 ChatMsg（不 mm_free 传入串），故用非拥有 WxString(std::wstring) 即可（同 send_text），无 double-free。
        constexpr uint32_t SEND_APPMSG = 0x1621280;  // appmsg 发送核心（ecx=ChatMsg，原始 XML 走 a4→content）
        constexpr uint32_t CHATMSG_CTOR = 0x1A0ED0;  // ChatMsg 默认构造器（__thiscall(this)→this，写 ??_7ChatMsg 与 InstanceCounter vftable，new(0x44) 建 +179 子对象）

        // send_emotion 发本地自定义表情。
        // 定位：CustomSmileyMgr 模块。发送叶子 sub_116C10C0 内嵌 9 处日志串 "CustomSmileyMgr::sendCustomEmotion" 锚定；
        //   EMO_MGR_GETTER=sub_1208250（CustomSmileyMgr Meyers 单例 getter，构造函数 sub_116BB450 写 ??_7CustomSmileyMgr@@6B@
        //   vftable 到 dword_1436A590，返回 &该对象本体）。旧汇编 `mov ebx,[EMO_CALL3]`（解引用全局指针）在目标版的对应物
        //   即此 getter——目标版全局是对象本体而非指针，故 manager=getter() 直接用、不再 deref（同 send_image）。
        // ABI（disasm 精确核对）：SEND_CUSTOM_EMOTION 为 __thiscall（ecx=manager），尾声 `mov esp,ebx;pop ebx;retn 5Ch`
        //   被调清栈 0x5C=92 字节=恰好 7 参（path + 空 + wxid + type=2 + 空 + 0 + buffer），故精确建模即栈平衡、无需帧指针纠正。
        //   参数与旧汇编逐字节吻合：[ebx+8/0xC]=path.ptr/len 为首参。type 常量取 2（本地文件来源）。
        // 所有权：函数尾声对传入的 path/wxid/两个空 WxString 的 .ptr 逐个 mm_free（[ebx+1C/28/30/3C/48/54]），
        //   故须用 ASSIGN 建 WeChat 拥有副本、按值传、绝不自行析构（同 send_file/richtext 模型，无 double-free）。
        // WxString 拷贝复用 RichText::ASSIGN(0x19DAF20)；buffer 为 0x1C 置零小结构（[+4]=size 置零走正常路径），非 ChatMsg、无 dtor。
        constexpr uint32_t EMO_MGR_GETTER      = 0x1208250;  // CustomSmileyMgr 单例 getter（返回 &对象本体 dword_1436A590）
        constexpr uint32_t SEND_CUSTOM_EMOTION = 0x6C10C0;   // CustomSmileyMgr::sendCustomEmotion 发送叶子（旧 EMO_CALL2 无效）
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
        // 低区（<0x48）不变；高区字段整体 +8（this+0x134 子对象内每个 std::string 后移 2 dword）。
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
    // 通过好友申请：走 AddFriendHelper（RTTI .?AVAddFriendHelper@@）。
    //   ACCEPT_CTOR = AddFriendHelper::AddFriendHelper（写 vftable 0x13BFA094，零初始化，注册 4 个事件处理器 sub_11803A10(179/182/177/178)）
    //   VERIFY_OK   = AddFriendHelper::VerifyOK（串 "AddFriendHelper::VerifyOK" 锚定；内部用 RichText::ASSIGN 把 v3 拷入 this+24；末尾 mm_free v4 的 wptr/ptr）
    //   ACCEPT_DTOR = AddFriendHelper::~AddFriendHelper（写 vftable，释放 this+6/this+9 的 WxString，复位 EventHandler vftable）
    // ABI（disasm 精确核对）：VerifyOK 是纯 __thiscall——ecx=buffer(this)，尾 `retn 30h` 被调清栈 0x30=48 字节=恰好 12 dword
    //   栈参：v3(&WxString) + nullbuffer + 0 + scratch(u64) + v4(WxStringValue 按值 5 dword) + scene + 0。被调清栈=精确 __thiscall 即栈平衡、无需帧指针纠正。
    // 所有权：v3 仅被读取(拷入 this+24)、不 free → 非拥有视图即可；v4 被 VerifyOK mm_free(wptr+ptr)，故须用 RichText::ASSIGN(0x19DAF20) 建 WeChat 拥有副本、按值传、勿自行析构。
    constexpr uint32_t ACCEPT_CTOR = 0x4CBD20;  // AddFriendHelper 构造器（原 ACCEPT_CALL1=0xA17D50）
    constexpr uint32_t VERIFY_OK   = 0x4CCE70;  // AddFriendHelper::VerifyOK（原 ACCEPT_CALL3=0xA18BD0）
    constexpr uint32_t ACCEPT_DTOR = 0x4CBE40;  // AddFriendHelper 析构器（原 ACCEPT_CALL4=0xA17E70）
} // namespace Friend

namespace Chatroom
{
    // 共用单例 getter：ChatRoomMgr magic-static（new(0x218)+ctor sub_1167DE60+register，返回对象指针本体）。
    // add/del/invite 三条链共用。
    constexpr uint32_t MGR_GETTER = 0x11C43A0;

    // ChatRoomMgr::doAddMemberToChatRoom（串 "ChatRoomMgr::doAddMemberToChatRoom" 锚定）。
    // 纯 __thiscall(ecx=manager, retn 0x20=8 栈参)；末尾 mm_free roomid 的 wptr/ptr → 需 ASSIGN 建拥有副本。
    // roomid 的 WxString::assign 复用 RichText::ASSIGN(0x19DAF20)。
    constexpr uint32_t ADD_MEMBER = 0x167F680;

    // ChatRoomMgr::doDelMemberFromChatRoom（串 "ChatRoomMgr::doDelMemberFromChatRoom" 锚定）。
    // aligned-stack __usercall prologue 但尾 retn 0x18=6 栈参被调清栈（members 1 + roomid 5），
    // args 读自 ebx 相对（原始栈）→ 从调用者看等价纯 __thiscall、栈平衡、无需帧指针纠正。
    // 末尾 mm_free roomid 的 wptr/ptr → 需 ASSIGN 建拥有副本；getter/assign 与加群共用。
    constexpr uint32_t DEL_MEMBER = 0x167FBD0;

    // 以下邀请业务入口仍是 3.9.2.23 旧值，待重定位（getter 已改用上面的 MGR_GETTER）
    constexpr uint32_t INV_CALL1 = 0x78CB40;
    constexpr uint32_t INV_CALL2 = 0x7F99D0;
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
    // 接收路径：
    //   CALL = SnsTimeLineMgr::OnSnsTimeLineSceneFinish 入口（朋友圈接收回调，__thiscall(this,a2,a3)，2 个调用者）；
    //   HOOK = OnProcessTimelineResp::<lambda_1> 内 `call OnSnsTimeLineSceneFinish(this,&container,0)` 定点，
    //          用返回地址 == HOOK+5 过滤，a2 即 dispatch 的容器指针。
    constexpr uint32_t HOOK    = 0x1FDB1E5;
    constexpr uint32_t CALL    = 0x1FDB4D0;
    // CALL1/2/3（单例 getter / 首页 / 翻页刷新）仅 refresh_pyq 用，仍是 3.9.2.23 旧值。
    constexpr uint32_t CALL1   = 0xC39680;
    constexpr uint32_t CALL2   = 0x14E2140;
    constexpr uint32_t CALL3   = 0x14E21E0;
    // 容器字段：主 feed 数组 begin/end 指针（容器 = a2）。
    constexpr uint32_t START   = 0x20;
    constexpr uint32_t END     = 0x24;
    // 单个 SnsObject（元素，STEP 字节）内字段：
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
    // 发送消息卡片（语义重命名）。
    // 业务对象是 MMReaderItem（RTTI .?AVMMReaderItem@@），管理器是 AppMsgMgr 全局单例。
    // 定位链：AppMsgMgr getter(sub_1076AE20)/send(sub_10B73000)/
    //   MMReaderItem ctor(sub_1076E630) → 经 BizProfileMsgBaseItem::ForwardtoUserNames(v31256
    //   sub_1119D480，构造 MMReaderItem 后走 getter→sub_11628DC0) 及字段读取器逐项对齐得 v31256 值。
    // 编排（无内联汇编，全部带类型 C++ 调用，所有权与 WeChat 内部一致）：
    //   CTOR(buff) → 用 ASSIGN 逐字段深拷贝 → GETTER()=manager → SEND(manager, receiver按值, buff) → DTOR(buff)
    // 注意：SEND 内部会 mm_free 传入的 receiver 字符串，故 receiver 必须用 ASSIGN 建 WeChat 拥有副本（勿传 std::wstring 别名）。
    constexpr uint32_t CTOR   = 0x11A0220;  // MMReaderItem::MMReaderItem（原 CALL1，__thiscall(this)，vftable ??_7MMReaderItem@@6B@）
    constexpr uint32_t GETTER = 0x119C990;  // AppMsgMgr 单例 getter（原 CALL2，magic-static，返回 &singleton）
    constexpr uint32_t ASSIGN = 0x19DAF20;  // WxString::assign(src,len)（__thiscall(this,src,len)，mm_realloc 深拷贝，原 CALL3 语义）
    constexpr uint32_t SEND   = 0x1628DC0;  // AppMsgMgr::sendAppMsg（原 CALL4，__thiscall(manager, WxString receiver 按值, MMReaderItem* buff)）
    constexpr uint32_t DTOR   = 0x119F530;  // MMReaderItem::~MMReaderItem 完整析构（原 CALL5，__thiscall(this)，释放全部成员+InstanceCounter 递减）

    // MMReaderItem 内 WxString 成员偏移（=该成员 wptr 的字节偏移）。低区不变、account/name 高区整体 +0x14（插入一个 WxString）。
    constexpr uint32_t F_TITLE    = 0x4;    // dword 1  标题（不变）
    constexpr uint32_t F_URL      = 0x2C;   // dword 11 链接（不变）
    constexpr uint32_t F_THUMBURL = 0x6C;   // dword 27 缩略图 URL（不变）
    constexpr uint32_t F_DIGEST   = 0x94;   // dword 37 摘要（不变）
    constexpr uint32_t F_ACCOUNT  = 0x1B4;  // dword 109 源用户名（原 0x1A0，+0x14）
    constexpr uint32_t F_NAME     = 0x1C8;  // dword 114 源显示名（原 0x1B4，+0x14）
    constexpr uint32_t OBJ_SIZE   = 0x280;  // MMReaderItem 栈缓冲大小（ctor 写至 dword158/0x278，取 WeChat 自身栈分配 v23[160]=0x280）
} // namespace RichText

namespace Pat
{
    // 拍一拍走 PatMgr。语义重命名。
    // MGR_GETTER = PatMgr magic-static 单例 getter（读 dword_1436A8C8，空则 new(0x6C)+ctor sub_11EB4620）。
    // SEND_PAT   = PatMgr::SendPatMsg（串锚定），__usercall：ecx=roomid(chat)、edx=wxid(patted)，
    //   3 个栈参（getter 结果/0/0）caller-clean（plain retn，调用点 add esp,0xC）；返回 al。
    constexpr uint32_t MGR_GETTER = 0x124A670;
    constexpr uint32_t SEND_PAT   = 0x1EB5D50;
} // namespace Pat

namespace OCR
{
    constexpr uint32_t CALL1 = 0x80A800;
    constexpr uint32_t CALL2 = 0x80F270;
    constexpr uint32_t CALL3 = 0x13DA3E0;
} // namespace OCR

namespace Forward
{
    // 转发消息。
    // 业务入口是 SendMessageMgr::forwordMsg：v3223 sub_10CE6730 → v31256 sub_11783230
    //   （靠串 "SendMessageMgr::forwordMsg"/"forward scene:%d msgid:%d" 锚定，结构 1:1：
    //    登录检查 +1420→+1468、内嵌 sub_11783C10=#13 SEND_MSG、msgPtr==0 时走
    //    sub_1166FE00=ChatMgr::GetMgrByPrefixLocalId 按 localId/dbIdx 加载消息）。
    // ABI __usercall，
    //   ecx=scene（转发场景，纯统计元数据，取 5）、edx=msgPtr（传 0 走加载路径），
    //   栈参：receiver WxString 按值(5 dword) + localId(u32) + dbIdx(u32)；返回 al；
    //   对齐栈 prologue(push ebx;mov ebx,esp;and esp,-8) → caller-clean，以 __fastcall 建模、
    //   栈失衡由 forward() 自身帧指针 epilogue 纠正。
    // receiver 用 RichText::ASSIGN(0x19DAF20) 建 WeChat 拥有副本；forwordMsg 末尾 mm_free 它。
    constexpr uint32_t FORWARD_MSG = 0x1783230;  // SendMessageMgr::forwordMsg（原 CALL2 语义）
} // namespace Forward

namespace QRCode
{
    constexpr uint32_t CALL1 = 0xAE9DB0;
    constexpr uint32_t CALL2 = 0xCDA6F0;
    constexpr uint32_t URL   = 0x3040DE8;
} // namespace QRCode

} // namespace Offsets
