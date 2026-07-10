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
using SendMgrGetterFn    = void *(*)();
using InitGlobalFn       = void (*)();
using SendTextFn         = int(__fastcall *)(void *buffer, const WxString *wxid, const WxString *msg,
                                             const std::vector<WxString> *at_wxids, uint32_t flag1,
                                             uint32_t zero1, uint32_t zero2, uint32_t zero3);
using SendImageFn        = void *(__thiscall *)(void *manager, void *buffer, const WxString *receiver,
                                                const WxString *path, void *options);
using SendFileFn         = void *(__thiscall *)(void *manager, void *buffer,
                                                WxStringValue receiver, WxStringValue path, int flag1,
                                                WxStringValue empty1, int flag2,
                                                WxStringValue empty2, int zero1, int zero2,
                                                WxStringValue empty3, int flag3, int flag4);
using RichTextManagerGetterFn = void *(*)();
using WxStringAssignFn   = void *(__thiscall *)(void *dest, const wchar_t *src, int len);
using SendRichTextFn     = int(__thiscall *)(void *manager, WxStringValue receiver, void *buffer);
using ChatMsgCtorFn      = void *(__thiscall *)(void *buffer);
using SendAppMsgXmlFn    = char(__fastcall *)(void *chatmsg, const WxString *from, const WxString *receiver,
                                              const WxString *content, const WxString *empty1,
                                              const WxString *path, int type, int flag,
                                              const WxString *empty2, int zero);
using ForwardMsgFn       = uint8_t(__fastcall *)(int scene, int msg_ptr, WxStringValue receiver,
                                                 uint32_t local_id, uint32_t db_idx);
using SendEmotionFn      = int(__thiscall *)(void *manager, WxStringValue path, WxStringValue null2,
                                             WxStringValue wxid, int type, WxStringValue null1,
                                             int zero, void *buffer);
using EmoMgrGetterFn     = void *(*)();

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

    auto init_global = reinterpret_cast<InitGlobalFn>(g_WeChatWinDllAddr + Offsets::Message::Send::SEND_MGR_GETTER);
    auto send_msg    = reinterpret_cast<SendTextFn>(g_WeChatWinDllAddr + Offsets::Message::Send::SEND_MSG);
    auto cleanup     = reinterpret_cast<BufferCleanupFn>(g_WeChatWinDllAddr + Offsets::Message::Send::CHATMSG_DTOR);

    std::wstring wsWxid = util::s2w(wxid);
    std::wstring wsMsg  = util::s2w(msg);
    WxString wxWxid(wsWxid);
    WxString wxMsg(wsMsg);

    // 先落全部 wstring 再包 WxString 视图，避免 vector 扩容使先前取的地址失效
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

    auto getter   = reinterpret_cast<SendMgrGetterFn>(g_WeChatWinDllAddr + Offsets::Message::Send::SEND_MGR_GETTER);
    auto send_img = reinterpret_cast<SendImageFn>(g_WeChatWinDllAddr + Offsets::Message::Send::SEND_IMAGE);
    auto cleanup  = reinterpret_cast<BufferCleanupFn>(g_WeChatWinDllAddr + Offsets::Message::Send::CHATMSG_DTOR);

    std::wstring wsWxid = util::s2w(wxid);
    std::wstring wsPath = util::s2w(path);
    WxString wxWxid(wsWxid);
    WxString wxPath(wsPath);

    // 普通图片：type=1，caption 为空，两个附加串（source/appinfo）指向空 WxString，其余全 0
    WxString extra1;
    WxString extra2;
    uint8_t options[0x40] = { 0 };
    *reinterpret_cast<uint32_t *>(options + 0x00) = 1;
    *reinterpret_cast<void **>(options + 0x2C)    = &extra1;
    *reinterpret_cast<void **>(options + 0x30)    = &extra2;

    char buffer[0x2D8] = { 0 };
    void *manager = getter();
    send_img(manager, buffer, &wxWxid, &wxPath, options);
    cleanup(buffer);
}

void send_file(const std::string &wxid, const std::string &path)
{
    if (g_WeChatWinDllAddr == 0) {
        LOG_ERROR("WeChatWin.dll not located.");
        return;
    }

    auto getter    = reinterpret_cast<RichTextManagerGetterFn>(g_WeChatWinDllAddr + Offsets::RichText::GETTER);
    auto assign    = reinterpret_cast<WxStringAssignFn>(g_WeChatWinDllAddr + Offsets::RichText::ASSIGN);
    auto send_file = reinterpret_cast<SendFileFn>(g_WeChatWinDllAddr + Offsets::Message::Send::SEND_FILE);
    auto cleanup   = reinterpret_cast<BufferCleanupFn>(g_WeChatWinDllAddr + Offsets::Message::Send::CHATMSG_DTOR);

    std::wstring wsWxid = util::s2w(wxid);
    std::wstring wsPath = util::s2w(path);

    // receiver/path 及三个空串都会被 sendFile mm_free，须用 ASSIGN 建 WeChat 拥有副本
    WxString wxReceiver;
    WxString wxPath;
    WxString wxEmpty1;
    WxString wxEmpty2;
    WxString wxEmpty3;
    assign(&wxReceiver, wsWxid.c_str(), -1);
    assign(&wxPath, wsPath.c_str(), -1);
    assign(&wxEmpty1, L"", -1);
    assign(&wxEmpty2, L"", -1);
    assign(&wxEmpty3, L"", -1);

    char buffer[0x2D8] = { 0 };
    void *manager      = getter();
    send_file(manager, buffer, to_value(wxReceiver), to_value(wxPath), 1, to_value(wxEmpty1), 0,
              to_value(wxEmpty2), 0, 0, to_value(wxEmpty3), 0, 0);
    cleanup(buffer);
}

void send_xml(const std::string &receiver, const std::string &xml, const std::string &path, int type)
{
    if (g_WeChatWinDllAddr == 0) {
        LOG_ERROR("WeChatWin.dll not located.");
        return;
    }

    // 发送核心会往 ChatMsg 字段 assign，故须先 CHATMSG_CTOR 真正构造、不能只置零；flag=1 跳过内部 <msgsource> 构造
    auto ctor = reinterpret_cast<ChatMsgCtorFn>(g_WeChatWinDllAddr + Offsets::Message::Send::CHATMSG_CTOR);
    auto send = reinterpret_cast<SendAppMsgXmlFn>(g_WeChatWinDllAddr + Offsets::Message::Send::SEND_APPMSG);
    auto dtor = reinterpret_cast<BufferCleanupFn>(g_WeChatWinDllAddr + Offsets::Message::Send::CHATMSG_DTOR);

    std::wstring wsFrom     = util::s2w(account::get_self_wxid());  // from 非空是发送核心的入口守卫要求
    std::wstring wsReceiver = util::s2w(receiver);
    std::wstring wsXml      = util::s2w(xml);
    std::wstring wsPath     = util::s2w(path);
    std::wstring wsEmpty;

    WxString wxFrom(wsFrom);
    WxString wxReceiver(wsReceiver);
    WxString wxXml(wsXml);
    WxString wxPath(wsPath);
    WxString wxEmpty1(wsEmpty);
    WxString wxEmpty2(wsEmpty);

    char buffer[0x500] = { 0 };
    ctor(buffer);
    send(buffer, &wxFrom, &wxReceiver, &wxXml, &wxEmpty1, &wxPath, type, 1, &wxEmpty2, 0);
    dtor(buffer);
}

void send_emotion(const std::string &wxid, const std::string &path)
{
    if (g_WeChatWinDllAddr == 0) {
        LOG_ERROR("WeChatWin.dll not located.");
        return;
    }

    // buffer 为 0x1C 置零小结构（[+4]=size 置零走"从文件发送"正常路径），非 ChatMsg、无 dtor
    auto getter = reinterpret_cast<EmoMgrGetterFn>(g_WeChatWinDllAddr + Offsets::Message::Send::EMO_MGR_GETTER);
    auto assign = reinterpret_cast<WxStringAssignFn>(g_WeChatWinDllAddr + Offsets::RichText::ASSIGN);
    auto send   = reinterpret_cast<SendEmotionFn>(g_WeChatWinDllAddr + Offsets::Message::Send::SEND_CUSTOM_EMOTION);

    std::wstring wsWxid = util::s2w(wxid);
    std::wstring wsPath = util::s2w(path);

    // path/wxid 及两个空串都会被 sendCustomEmotion mm_free，须用 ASSIGN 建 WeChat 拥有副本
    WxString wxPath;
    WxString wxWxid;
    WxString wxEmpty1;
    WxString wxEmpty2;
    assign(&wxPath, wsPath.c_str(), -1);
    assign(&wxWxid, wsWxid.c_str(), -1);
    assign(&wxEmpty1, L"", -1);
    assign(&wxEmpty2, L"", -1);

    char buffer[0x1C] = { 0 };
    void *manager     = getter();
    send(manager, to_value(wxPath), to_value(wxEmpty1), to_value(wxWxid), 2, to_value(wxEmpty2), 0, buffer);
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

    // 收件人须为 WeChat 拥有副本（SEND 内部 mm_free）
    WxString wxReceiver;
    assign(&wxReceiver, wsReceiver.c_str(), -1);

    void *manager = getter();
    int status    = send(manager, to_value(wxReceiver), buff);
    cleanup(buff);

    return status;
}

int send_pat(const std::string &roomid, const std::string &wxid)
{
    if (g_WeChatWinDllAddr == 0) {
        LOG_ERROR("WeChatWin.dll not located.");
        return -1;
    }

    // getter 返回值仅作 SendPatMsg 的 SEH 帧 token 透传（不解引用）；两个废弃栈参用 0 占位
    auto getter       = reinterpret_cast<PatManagerGetterFn>(g_WeChatWinDllAddr + Offsets::Pat::MGR_GETTER);
    auto send_pat_msg = reinterpret_cast<SendPatFn>(g_WeChatWinDllAddr + Offsets::Pat::SEND_PAT);

    std::wstring wsRoomid = util::s2w(roomid);
    std::wstring wsWxid   = util::s2w(wxid);
    WxString wxRoomid(wsRoomid);
    WxString wxWxid(wsWxid);

    void *manager  = getter();
    uint8_t status = send_pat_msg(&wxRoomid, &wxWxid, manager, 0, 0);

    return status ? 0 : -1;
}

int forward(uint64_t msgid, const std::string &receiver)
{
    if (g_WeChatWinDllAddr == 0) {
        LOG_ERROR("WeChatWin.dll not located.");
        return -1;
    }

    // 由 MsgSvrID 反查进程内 (localId, dbIdx)
    uint64_t localId = 0;
    uint32_t dbIdx   = 0;
    if (db::get_local_id_and_dbidx(msgid, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", std::to_string(msgid));
        return -1;
    }

    // msg_ptr 传 0 → 内部按 (localId, dbIdx) 加载源消息；scene=5 仅作转发场景统计元数据。
    // 收件人须为 WeChat 拥有副本（forwordMsg 末尾 mm_free 它），用 ASSIGN 深拷贝。
    auto assign      = reinterpret_cast<WxStringAssignFn>(g_WeChatWinDllAddr + Offsets::RichText::ASSIGN);
    auto forward_msg = reinterpret_cast<ForwardMsgFn>(g_WeChatWinDllAddr + Offsets::Forward::FORWARD_MSG);

    std::wstring wsReceiver = util::s2w(receiver);
    WxString wxReceiver;
    assign(&wxReceiver, wsReceiver.c_str(), -1);

    uint8_t status = forward_msg(5, 0, to_value(wxReceiver), static_cast<uint32_t>(localId), dbIdx);

    return status ? 0 : -1;
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
