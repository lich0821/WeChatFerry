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
using ImageSessionGetterFn = void **(*)();
using InitGlobalFn       = void (*)();
using SendTextFn         = int(__fastcall *)(void *buffer, const WxString *wxid, const WxString *msg,
                                             const std::vector<WxString> *at_wxids, uint32_t flag1,
                                             uint32_t zero1, uint32_t zero2, uint32_t zero3);
using SendImageFn        = int(__thiscall *)(void *manager, void *buffer, const WxString *wxid,
                                             const WxString *path, WxStringValue null_value);
using FileSessionGetterFn = void **(*)();
using SendFileFn         = void *(__thiscall *)(void *manager, void *buffer, WxStringValue wxid,
                                                WxStringValue path, WxStringValue null_value, int reserved);
using RichTextManagerGetterFn = void *(*)();
using SendRichTextFn     = int(__thiscall *)(void *manager, void *buffer, WxStringValue receiver);
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
    (void)wxid;
    (void)path;
    LOG_ERROR("Not Implemented yet.");
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
    (void)rt;
    LOG_ERROR("Not Implemented yet.");
    return -1;
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
