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

struct RichTextData {
    std::string name;
    std::string account;
    std::string title;
    std::string digest;
    std::string url;
    std::string thumburl;
    std::string receiver;
};

} // namespace

namespace message
{

void send_text(const std::string &wxid, const std::string &msg, const std::string &at_wxids)
{
    int success        = 0;
    char buffer[0x2D8] = { 0 };

    uint32_t sendCall1 = g_WeChatWinDllAddr + Offsets::Message::Send::TEXT_CALL1;
    uint32_t sendCall2 = g_WeChatWinDllAddr + Offsets::Message::Send::TEXT_CALL2;
    uint32_t sendCall3 = g_WeChatWinDllAddr + Offsets::Message::Send::TEXT_CALL3;

    std::wstring wsWxid = util::s2w(wxid);
    std::wstring wsMsg  = util::s2w(msg);
    WxString wxMsg(wsMsg);
    WxString wxWxid(wsWxid);

    std::vector<std::wstring> vAtWxids;
    std::vector<WxString> vWxAtWxids;
    if (!at_wxids.empty()) {
        std::wstringstream wss(util::s2w(at_wxids));
        while (wss.good()) {
            std::wstring wstr;
            getline(wss, wstr, L',');
            vAtWxids.push_back(wstr);
            WxString wxAtWxid(vAtWxids.back());
            vWxAtWxids.push_back(wxAtWxid);
        }
    }

    __asm
    {
        pushad;
        call sendCall1;
        push 0x0;
        push 0x0;
        push 0x0;
        push 0x1;
        lea eax, vWxAtWxids;
        push eax;
        lea eax, wxMsg;
        push eax;
        lea edx, wxWxid;
        lea ecx, buffer;
        call sendCall2;
        mov success, eax;
        add esp, 0x18;
        lea ecx, buffer;
        call sendCall3;
        popad;
    }
}

void send_image(const std::string &wxid, const std::string &path)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }
    int success      = 0;
    uint32_t tmpEAX  = 0;
    char buf[0x2D8]  = { 0 };

    std::wstring wsWxid = util::s2w(wxid);
    std::wstring wsPath = util::s2w(path);

    WxString wxWxid(wsWxid);
    WxString wxPath(wsPath);
    WxString nullbuffer;

    uint32_t sendCall1 = g_WeChatWinDllAddr + Offsets::Message::Send::IMG_CALL1;
    uint32_t sendCall2 = g_WeChatWinDllAddr + Offsets::Message::Send::IMG_CALL2;
    uint32_t sendCall3 = g_WeChatWinDllAddr + Offsets::Message::Send::IMG_CALL3;
    uint32_t sendCall4 = g_WeChatWinDllAddr + Offsets::Message::Send::IMG_CALL4;

    __asm {
        pushad;
        call       sendCall1;
        sub        esp,0x14;
        mov        tmpEAX,eax;
        lea        eax,nullbuffer;
        mov        ecx,esp;
        lea        edi,wxPath;
        push       eax;
        call       sendCall2;
        mov        ecx,dword ptr [tmpEAX];
        lea        eax,wxWxid;
        push       edi;
        push       eax;
        lea        eax,buf;
        push       eax;
        call       sendCall3;
        mov        success,eax;
        lea        ecx,buf;
        call       sendCall4;
        popad;
    }
}

void send_file(const std::string &wxid, const std::string &path)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }
    int success         = 0;
    uint32_t tmpEAX     = 0;
    char buffer[0x2D8]  = { 0 };

    std::wstring wsWxid = util::s2w(wxid);
    std::wstring wsPath = util::s2w(path);

    WxString wxWxid(wsWxid);
    WxString wxPath(wsPath);
    WxString nullbuffer;

    uint32_t sendCall1 = g_WeChatWinDllAddr + Offsets::Message::Send::FILE_CALL1;
    uint32_t sendCall2 = g_WeChatWinDllAddr + Offsets::Message::Send::FILE_CALL2;
    uint32_t sendCall3 = g_WeChatWinDllAddr + Offsets::Message::Send::FILE_CALL3;
    uint32_t sendCall4 = g_WeChatWinDllAddr + Offsets::Message::Send::FILE_CALL4;

    __asm {
        pushad;
        pushfd;
        call sendCall1;
        sub esp, 0x14;
        mov tmpEAX, eax;
        lea eax, nullbuffer;
        mov ecx, esp;
        push eax;
        call sendCall2;
        push 0x0;
        sub esp, 0x14;
        mov edi, esp;
        mov dword ptr[edi], 0;
        mov dword ptr[edi + 0x4], 0;
        mov dword ptr[edi + 0x8], 0;
        mov dword ptr[edi + 0xc], 0;
        mov dword ptr[edi + 0x10], 0;
        sub esp, 0x14;
        lea eax, wxPath;
        mov ecx, esp;
        push eax;
        call sendCall2;
        sub esp, 0x14;
        lea eax, wxWxid;
        mov ecx, esp;
        push eax;
        call sendCall2;
        mov ecx, dword ptr[tmpEAX];
        lea eax, buffer;
        push eax;
        call sendCall3;
        mov al, byte ptr[eax + 0x38];
        movzx eax, al;
        mov success, eax;
        lea ecx, buffer;
        call sendCall4;
        popfd;
        popad;
    }
}

void send_xml(const std::string &receiver, const std::string &xml, const std::string &path, int type)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }

    uint32_t sendXmlCall1 = g_WeChatWinDllAddr + Offsets::Message::Send::XML_CALL1;
    uint32_t sendXmlCall2 = g_WeChatWinDllAddr + Offsets::Message::Send::XML_CALL2;
    uint32_t sendXmlCall3 = g_WeChatWinDllAddr + Offsets::Message::Send::XML_CALL3;
    uint32_t sendXmlCall4 = g_WeChatWinDllAddr + Offsets::Message::Send::XML_CALL4;
    uint32_t sendXmlParam = g_WeChatWinDllAddr + Offsets::Message::Send::XML_PARAM;

    char buffer[0xFF0] = { 0 };
    char nullBuf[0x1C] = { 0 };

    std::wstring wsSender   = util::s2w(account::get_self_wxid());
    std::wstring wsReceiver = util::s2w(receiver);
    std::wstring wsXml      = util::s2w(xml);

    WxString wxPath;
    WxString wxNull;
    WxString wxXml(wsXml);
    WxString wxSender(wsSender);
    WxString wxReceiver(wsReceiver);

    if (!path.empty()) {
        std::wstring wsPath = util::s2w(path);
        wxPath              = WxString(wsPath);
    }

    uint32_t sendtype = type;
    __asm {
        pushad;
        pushfd;
        lea ecx, buffer;
        call sendXmlCall1;
        mov eax, [sendtype];
        push eax;
        lea eax, nullBuf;
        lea edx, wxSender;
        push eax;
        lea eax, wxPath;
        push eax;
        lea eax, wxXml;
        push eax;
        lea edi, wxReceiver;
        push edi;
        lea ecx, buffer;
        call sendXmlCall2;
        add esp, 0x14;
        lea eax, wxNull;
        push eax;
        lea ecx, buffer;
        call sendXmlCall3;
        mov dl, 0x0;
        lea ecx, buffer;
        push sendXmlParam;
        push sendXmlParam;
        call sendXmlCall4;
        add esp, 0x8;
        popfd;
        popad;
    }
}

void send_emotion(const std::string &wxid, const std::string &path)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }

    char buffer[0x1C] = { 0 };
    std::wstring wsWxid = util::s2w(wxid);
    std::wstring wsPath = util::s2w(path);

    WxString wxWxid(wsWxid);
    WxString wxPath(wsPath);
    WxString nullbuffer;

    uint32_t sendCall1 = g_WeChatWinDllAddr + Offsets::Message::Send::EMO_CALL1;
    uint32_t sendCall2 = g_WeChatWinDllAddr + Offsets::Message::Send::EMO_CALL2;
    uint32_t sendCall3 = g_WeChatWinDllAddr + Offsets::Message::Send::EMO_CALL3;

    __asm {
        pushad;
        pushfd;
        mov ebx, dword ptr[sendCall3];
        lea eax, buffer;
        push eax;
        push 0x0;
        sub esp, 0x14;
        mov esi, esp;
        mov dword ptr [esi], 0x0;
        mov dword ptr [esi+0x4], 0x0;
        mov dword ptr [esi+0x8], 0x0;
        mov dword ptr [esi+0xC], 0x0;
        mov dword ptr [esi+0x10], 0x0;
        push 0x2;
        lea eax, wxWxid;
        sub esp, 0x14;
        mov ecx, esp;
        push eax;
        call sendCall1;
        sub esp, 0x14;
        mov esi, esp;
        mov dword ptr [esi], 0x0;
        mov dword ptr [esi+0x4], 0x0;
        mov dword ptr [esi+0x8], 0x0;
        mov dword ptr [esi+0xC], 0x0;
        mov dword ptr [esi+0x10], 0x0;
        sub esp, 0x14;
        mov ecx, esp;
        lea eax, wxPath;
        push eax;
        call sendCall1;
        mov ecx, ebx;
        call sendCall2;
        popfd;
        popad;
    }
}

int send_rich_text(const RichText &rt)
{
    RichTextData richText;
    richText.name     = rt.name ? rt.name : "";
    richText.account  = rt.account ? rt.account : "";
    richText.title    = rt.title ? rt.title : "";
    richText.digest   = rt.digest ? rt.digest : "";
    richText.url      = rt.url ? rt.url : "";
    richText.thumburl = rt.thumburl ? rt.thumburl : "";
    richText.receiver = rt.receiver ? rt.receiver : "";

    int status       = -1;
    char buff[0x238] = { 0 };

    uint32_t rtCall3 = g_WeChatWinDllAddr + Offsets::RichText::CALL3;
    uint32_t rtCall2 = g_WeChatWinDllAddr + Offsets::RichText::CALL2;
    uint32_t rtCall1 = g_WeChatWinDllAddr + Offsets::RichText::CALL1;
    uint32_t rtCall5 = g_WeChatWinDllAddr + Offsets::RichText::CALL5;
    uint32_t rtCall4 = g_WeChatWinDllAddr + Offsets::RichText::CALL4;

    __asm {
        pushad;
        pushfd;
        lea ecx,buff;
        call rtCall1;
        popfd;
        popad;
    }

    std::wstring receiver = util::s2w(richText.receiver);
    std::wstring title    = util::s2w(richText.title);
    std::wstring url      = util::s2w(richText.url);
    std::wstring thumburl = util::s2w(richText.thumburl);
    std::wstring account  = util::s2w(richText.account);
    std::wstring name     = util::s2w(richText.name);
    std::wstring digest   = util::s2w(richText.digest);

    WxString wxReceiver(receiver);
    WxString wxTitle(title);
    WxString wxUrl(url);
    WxString wxThumburl(thumburl);
    WxString wxAccount(account);
    WxString wxName(name);
    WxString wxDigest(digest);

    memcpy(&buff[0x4], &wxTitle, sizeof(wxTitle));
    memcpy(&buff[0x2C], &wxUrl, sizeof(wxUrl));
    memcpy(&buff[0x6C], &wxThumburl, sizeof(wxThumburl));
    memcpy(&buff[0x94], &wxDigest, sizeof(wxDigest));
    memcpy(&buff[0x1A0], &wxAccount, sizeof(wxAccount));
    memcpy(&buff[0x1B4], &wxName, sizeof(wxName));

    __asm {
        pushad;
        pushfd;
        call rtCall2;
        lea ecx, buff;
        push ecx;
        sub esp, 0x14;
        mov edi, eax;
        mov ecx, esp;
        lea ebx, wxReceiver;
        push ebx;
        call rtCall3;
        mov ecx, edi;
        call rtCall4;
        mov status, eax;
        add ebx, 0x14;
        lea ecx, buff;
        push 0x0;
        call rtCall5;
        popfd;
        popad;
    }

    return status;
}

int send_pat(const std::string &roomid, const std::string &wxid)
{
    int status = -1;

    std::wstring wsRoomid = util::s2w(roomid);
    std::wstring wsWxid   = util::s2w(wxid);
    WxString wxRoomid(wsRoomid);
    WxString wxWxid(wsWxid);

    uint32_t pmCall1 = g_WeChatWinDllAddr + Offsets::Pat::CALL1;
    uint32_t pmCall2 = g_WeChatWinDllAddr + Offsets::Pat::CALL2;
    uint32_t pmCall3 = g_WeChatWinDllAddr + Offsets::Pat::CALL3;

    __asm {
        pushad;
        call  pmCall1;
        push  pmCall2;
        push  0x0;
        push  eax;
        lea   ecx, wxRoomid;
        lea   edx, wxWxid;
        call  pmCall3;
        add   esp, 0xc;
        movzx eax, al;
        mov   status, eax;
        popad;
    }

    return status;
}

int forward(uint64_t msgid, const std::string &receiver)
{
    int status       = -1;
    uint32_t dbIdx   = 0;
    uint64_t localId = 0;

    if (db::get_local_id_and_dbidx(msgid, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", to_string(msgid));
        return status;
    }

    std::wstring wsReceiver = util::s2w(receiver);
    WxString wxReceiver(wsReceiver);

    uint32_t fmCall1 = g_WeChatWinDllAddr + Offsets::Forward::CALL1;
    uint32_t fmCall2 = g_WeChatWinDllAddr + Offsets::Forward::CALL2;

    __asm {
        pushad;
        pushfd;
        mov        edx, dword ptr [dbIdx];
        push       edx;
        mov        eax, dword ptr [localId];
        push       eax;
        sub        esp, 0x14;
        mov        ecx, esp;
        lea        esi, wxReceiver;
        push       esi;
        call       fmCall1;
        xor        ecx, ecx;
        call       fmCall2;
        movzx      eax, al;
        mov        status, eax;
        add        esp, 0x1c;
        popfd;
        popad;
    }

    return status;
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
        int status      = send_rich_text(rt);
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
