#include "message_sender.h"

#include <filesystem>
#include <sstream>
#include <vector>

#include "account_manager.h"
#include "database_executor.h"
#include "framework.h"
#include "log.hpp"
#include "offsets.h"
#include "rpc_helper.h"
#include "spy.h"
#include "spy_types.h"
#include "util.h"

namespace fs = std::filesystem;

namespace
{

struct WxStringValue {
    const wchar_t *wptr;
    uint32_t size;
    uint32_t capacity;
    const char *ptr;
    uint32_t clen;
};

using PatManagerGetterFn = void *(*)();
using SendPatFn          = uint8_t(__fastcall *)(WxString *roomid, WxString *wxid, void *manager,
                                                 uint32_t reserved1, uint32_t reserved2);
using BufferInitFn       = void(__thiscall *)(void *buffer);
using BufferCleanupFn    = void(__thiscall *)(void *buffer);
using BufferCleanupExFn  = void(__thiscall *)(void *buffer, int free_memory);
using SendMgrGetterFn    = void *(*)();  // SendMessageMgr 单例 getter（无参，返回 manager*）
using InitGlobalFn       = void (*)();
using SendTextFn         = int(__fastcall *)(void *buffer, const WxString *wxid, const WxString *msg,
                                             const std::vector<WxString> *at_wxids, uint32_t flag1,
                                             uint32_t zero1, uint32_t zero2, uint32_t zero3);
// SendMessageMgr 图片提交叶子（sub_11783120，__thiscall）：
//   ecx=manager, buf=输出 ChatMsg, receiver/path=WxString*, options=选项结构指针（布局见 send_image）
using SendImageFn        = void *(__thiscall *)(void *manager, void *buffer, const WxString *receiver,
                                                const WxString *path, void *options);
using FileSessionGetterFn = void **(*)();
using SendFileFn         = void *(__thiscall *)(void *manager, void *buffer, WxStringValue wxid,
                                                WxStringValue path, WxStringValue null_value, int reserved);
using RichTextManagerGetterFn = void *(*)();
// WxString::assign(src,len)（__thiscall(this,src,len)，mm_realloc 深拷贝，返回值忽略）
using WxStringAssignFn   = void *(__thiscall *)(void *dest, const wchar_t *src, int len);
// AppMsgMgr::sendAppMsg：__thiscall，ecx=manager，收件人 WxString 按值在前、MMReaderItem* buff 在后
using SendRichTextFn     = int(__thiscall *)(void *manager, WxStringValue receiver, void *buffer);
using SendXmlBuildFn     = int(__fastcall *)(void *buffer, const WxString *sender, const WxString *receiver,
                                             const WxString *xml, const WxString *path, void *null_buf, int type);
using SendXmlFinalizeFn  = void(__thiscall *)(void *buffer, const WxString *null_obj);
using SendXmlCommitFn    = int(__fastcall *)(void *buffer, int zero, uint32_t param1, uint32_t param2);
using PrepareForwardFn   = int(__fastcall *)(void *scratch, uint32_t dbidx_reg, const WxString *receiver,
                                             uint32_t local_id, uint32_t dbidx_stack);
using ExecuteForwardFn   = uint8_t(__thiscall *)(void *self);
using SendEmotionFn      = int(__thiscall *)(void *manager, WxStringValue path, WxStringValue null2,
                                             WxStringValue wxid, int type, WxStringValue null1,
                                             int zero, void *buffer);

struct RichTextData {
    std::string name;
    std::string account;
    std::string title;
    std::string digest;
    std::string url;
    std::string thumburl;
    std::string receiver;
};

WxStringValue to_value(const WxString &value)
{
    return { value.wptr, value.size, value.capacity, value.ptr, value.clen };
}

} // namespace

namespace message
{

void send_text(const std::string &wxid, const std::string &msg, const std::string &at_wxids)
{
    if (g_WeChatWinDllAddr == 0) {
        LOG_ERROR("WeChatWin.dll not located.");
        return;
    }

    // 三步编排（等价旧内联汇编，全部建模为带类型的 C++ 调用）：
    // 1) SEND_MGR_GETTER：SendMessageMgr 单例 getter（无参，返回值忽略，仅确保单例已初始化）
    // 2) SEND_MSG：SendMessageMgr::sendMsg（__fastcall：ecx=buffer, edx=wxid，栈参 msg/at/flag=1/0/0/0）
    // 3) CHATMSG_DTOR：ChatMsg::~ChatMsg，清理 buffer（该对象恰好 0x2D8 字节）
    auto init_global = reinterpret_cast<InitGlobalFn>(g_WeChatWinDllAddr + Offsets::Message::Send::SEND_MGR_GETTER);
    auto send_msg    = reinterpret_cast<SendTextFn>(g_WeChatWinDllAddr + Offsets::Message::Send::SEND_MSG);
    auto cleanup     = reinterpret_cast<BufferCleanupFn>(g_WeChatWinDllAddr + Offsets::Message::Send::CHATMSG_DTOR);

    std::wstring wsWxid = util::s2w(wxid);
    std::wstring wsMsg  = util::s2w(msg);
    WxString wxWxid(wsWxid);
    WxString wxMsg(wsMsg);

    // @ 列表：先把所有 wstring 落进 vAtWxids（稳定后再取地址包成 WxString，避免 vector 扩容使指针失效）
    std::vector<std::wstring> vAtWxids;
    std::vector<WxString> vWxAtWxids;
    if (!at_wxids.empty()) {
        std::wstringstream wss(util::s2w(at_wxids));
        std::wstring wstr;
        while (std::getline(wss, wstr, L',')) {
            if (!wstr.empty()) {
                vAtWxids.push_back(wstr);
            }
        }
        vWxAtWxids.reserve(vAtWxids.size());
        for (std::wstring &w : vAtWxids) {
            vWxAtWxids.push_back(WxString(w));
        }
    }

    char buffer[0x2D8] = { 0 };
    init_global();
    send_msg(buffer, &wxWxid, &wxMsg, &vWxAtWxids, 1, 0, 0, 0);
    cleanup(buffer);
}

void send_image(const std::string &wxid, const std::string &path)
{
    if (g_WeChatWinDllAddr == 0) {
        LOG_ERROR("WeChatWin.dll not located.");
        return;
    }

    // 三步编排（等价旧内联汇编，全部建模为带类型的 C++ 调用）：
    // 1) SEND_MGR_GETTER：SendMessageMgr 单例 getter（返回值即 manager，直接作 leaf 的 this）
    // 2) SEND_IMAGE：SendMessageMgr 图片提交叶子（__thiscall：ecx=manager, buf/receiver/path/options）
    // 3) CHATMSG_DTOR：ChatMsg::~ChatMsg，清理栈上临时 ChatMsg（对象恰好 0x2D8 字节）
    auto getter   = reinterpret_cast<SendMgrGetterFn>(g_WeChatWinDllAddr + Offsets::Message::Send::SEND_MGR_GETTER);
    auto send_img = reinterpret_cast<SendImageFn>(g_WeChatWinDllAddr + Offsets::Message::Send::SEND_IMAGE);
    auto cleanup  = reinterpret_cast<BufferCleanupFn>(g_WeChatWinDllAddr + Offsets::Message::Send::CHATMSG_DTOR);

    std::wstring wsWxid = util::s2w(wxid);
    std::wstring wsPath = util::s2w(path);
    WxString wxWxid(wsWxid);
    WxString wxPath(wsPath);

    // options 选项结构（镜像 ChatViewModel::reSendMsg(sub_113CE240) case 3 的 v53）。
    // 叶子 sub_11783120 读取的字段（相对 options）：
    //   +0x00 type、+0x04/+0x10/+0x14 保留、+0x18 caption wchar*(0→空)、+0x1C caption len、
    //   +0x2C/+0x30 两个 WxString* 附加串（source/appinfo，普通发图传空串）。
    // 普通图片：type=1，caption 为空，两个附加串指向空 WxString，其余全 0。
    WxString extra1;
    WxString extra2;
    uint8_t options[0x40] = { 0 };
    *reinterpret_cast<uint32_t *>(options + 0x00) = 1;              // type = 1（普通图片）
    *reinterpret_cast<void **>(options + 0x2C)    = &extra1;        // 附加串 1（空）
    *reinterpret_cast<void **>(options + 0x30)    = &extra2;        // 附加串 2（空）

    char buffer[0x2D8] = { 0 };
    void *manager = getter();
    send_img(manager, buffer, &wxWxid, &wxPath, options);
    cleanup(buffer);
}

void send_file(const std::string &wxid, const std::string &path)
{
    (void)wxid;
    (void)path;
    LOG_ERROR("Not Implemented yet.");
}

void send_xml(const std::string &receiver, const std::string &xml, const std::string &path, int type)
{
    (void)receiver;
    (void)xml;
    (void)path;
    (void)type;
    LOG_ERROR("Not Implemented yet.");
}

void send_emotion(const std::string &wxid, const std::string &path)
{
    (void)wxid;
    (void)path;
    LOG_ERROR("Not Implemented yet.");
}

int send_rich_text(const RichText &rt)
{
    if (g_WeChatWinDllAddr == 0) {
        LOG_ERROR("WeChatWin.dll not located.");
        return -1;
    }

    RichTextData data;
    data.name     = rt.name ? rt.name : "";
    data.account  = rt.account ? rt.account : "";
    data.title    = rt.title ? rt.title : "";
    data.digest   = rt.digest ? rt.digest : "";
    data.url      = rt.url ? rt.url : "";
    data.thumburl = rt.thumburl ? rt.thumburl : "";
    data.receiver = rt.receiver ? rt.receiver : "";

    if (data.receiver.empty()) {
        LOG_ERROR("Empty receiver.");
        return -1;
    }

    // 五步编排（等价旧内联汇编，全部建模为带类型 C++ 调用，所有权与 WeChat 内部一致）：
    // 1) CTOR：MMReaderItem 构造（__thiscall(this=buff)，清零全部字段并写 vftable）
    // 2) ASSIGN：逐字段深拷贝进 MMReaderItem（WeChat mm_alloc 拥有，由 DTOR 释放）
    // 3) GETTER：AppMsgMgr 单例（返回值即 manager）
    // 4) SEND：AppMsgMgr::sendAppMsg（__thiscall(manager, receiver 按值, buff)）——内部会 mm_free receiver
    // 5) DTOR：MMReaderItem 完整析构（释放各字段 + InstanceCounter 递减）
    auto ctor    = reinterpret_cast<BufferInitFn>(g_WeChatWinDllAddr + Offsets::RichText::CTOR);
    auto assign  = reinterpret_cast<WxStringAssignFn>(g_WeChatWinDllAddr + Offsets::RichText::ASSIGN);
    auto getter  = reinterpret_cast<RichTextManagerGetterFn>(g_WeChatWinDllAddr + Offsets::RichText::GETTER);
    auto send    = reinterpret_cast<SendRichTextFn>(g_WeChatWinDllAddr + Offsets::RichText::SEND);
    auto cleanup = reinterpret_cast<BufferCleanupFn>(g_WeChatWinDllAddr + Offsets::RichText::DTOR);

    std::wstring wsTitle    = util::s2w(data.title);
    std::wstring wsUrl      = util::s2w(data.url);
    std::wstring wsThumburl = util::s2w(data.thumburl);
    std::wstring wsDigest   = util::s2w(data.digest);
    std::wstring wsAccount  = util::s2w(data.account);
    std::wstring wsName     = util::s2w(data.name);
    std::wstring wsReceiver = util::s2w(data.receiver);

    char buff[Offsets::RichText::OBJ_SIZE] = { 0 };
    ctor(buff);
    assign(buff + Offsets::RichText::F_TITLE, wsTitle.c_str(), -1);
    assign(buff + Offsets::RichText::F_URL, wsUrl.c_str(), -1);
    assign(buff + Offsets::RichText::F_THUMBURL, wsThumburl.c_str(), -1);
    assign(buff + Offsets::RichText::F_DIGEST, wsDigest.c_str(), -1);
    assign(buff + Offsets::RichText::F_ACCOUNT, wsAccount.c_str(), -1);
    assign(buff + Offsets::RichText::F_NAME, wsName.c_str(), -1);

    // 收件人须为 WeChat 拥有副本（SEND 内部 mm_free），不能传 std::wstring 别名
    WxString wxReceiver;
    assign(&wxReceiver, wsReceiver.c_str(), -1);

    void *manager = getter();
    int status    = send(manager, to_value(wxReceiver), buff);
    cleanup(buff);

    return status;
}

int send_pat(const std::string &roomid, const std::string &wxid)
{
    (void)roomid;
    (void)wxid;
    LOG_ERROR("Not Implemented yet.");
    return -1;
}

int forward(uint64_t msgid, const std::string &receiver)
{
    (void)msgid;
    (void)receiver;
    LOG_ERROR("Not Implemented yet.");
    return -1;
}

bool rpc_send_text(const TextMsg &text, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_SEND_TXT>(out, len, [&text](Response &rsp) {
        if ((text.msg == NULL) || (text.receiver == NULL)) {
            LOG_ERROR("Empty message or receiver.");
            rsp.msg.status = -1;
        } else {
            std::string msg(text.msg);
            std::string receiver(text.receiver);
            std::string aters(text.aters ? text.aters : "");
            send_text(receiver, msg, aters);
            rsp.msg.status = 0;
        }
    });
}

bool rpc_send_image(const PathMsg &file, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_SEND_IMG>(out, len, [&file](Response &rsp) {
        if ((file.path == NULL) || (file.receiver == NULL)) {
            LOG_ERROR("Empty path or receiver.");
            rsp.msg.status = -1;
        } else if (!fs::exists(file.path)) {
            LOG_ERROR("Path does not exist: {}", file.path);
            rsp.msg.status = -2;
        } else {
            send_image(file.receiver, file.path);
            rsp.msg.status = 0;
        }
    });
}

bool rpc_send_file(const PathMsg &file, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_SEND_FILE>(out, len, [&file](Response &rsp) {
        if ((file.path == NULL) || (file.receiver == NULL)) {
            LOG_ERROR("Empty path or receiver.");
            rsp.msg.status = -1;
        } else if (!fs::exists(file.path)) {
            LOG_ERROR("Path does not exist: {}", file.path);
            rsp.msg.status = -2;
        } else {
            send_file(file.receiver, file.path);
            rsp.msg.status = 0;
        }
    });
}

bool rpc_send_emotion(const PathMsg &file, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_SEND_EMOTION>(out, len, [&file](Response &rsp) {
        if ((file.path == NULL) || (file.receiver == NULL)) {
            LOG_ERROR("Empty path or receiver.");
            rsp.msg.status = -1;
        } else if (!fs::exists(file.path)) {
            LOG_ERROR("Path does not exist: {}", file.path);
            rsp.msg.status = -2;
        } else {
            send_emotion(file.receiver, file.path);
            rsp.msg.status = 0;
        }
    });
}

bool rpc_send_xml(const XmlMsg &xml, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_SEND_XML>(out, len, [&xml](Response &rsp) {
        if ((xml.content == NULL) || (xml.receiver == NULL)) {
            LOG_ERROR("Empty content or receiver.");
            rsp.msg.status = -1;
        } else {
            std::string content(xml.content);
            std::string receiver(xml.receiver);
            std::string path(xml.path ? xml.path : "");
            int type = xml.type;
            send_xml(receiver, content, path, type);
            rsp.msg.status = 0;
        }
    });
}

bool rpc_send_rich_text(const RichText &rt, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_SEND_RICH_TXT>(out, len, [&rt](Response &rsp) {
        int status     = send_rich_text(rt);
        rsp.msg.status = status;
    });
}

bool rpc_send_pat(const PatMsg &pat, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_SEND_PAT_MSG>(out, len, [&pat](Response &rsp) {
        if ((pat.roomid == NULL) || (pat.wxid == NULL)) {
            LOG_ERROR("Empty roomid or wxid.");
            rsp.msg.status = -1;
        } else {
            int status = send_pat(pat.roomid, pat.wxid);
            rsp.msg.status = status;
        }
    });
}

bool rpc_forward(const ForwardMsg &fm, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_FORWARD_MSG>(out, len, [&fm](Response &rsp) {
        if (fm.receiver == NULL) {
            LOG_ERROR("Empty receiver.");
            rsp.msg.status = -1;
        } else {
            int status = forward(fm.id, fm.receiver);
            rsp.msg.status = status;
        }
    });
}

} // namespace message
