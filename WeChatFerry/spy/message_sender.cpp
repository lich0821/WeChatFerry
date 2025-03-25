#include "message_sender.h"

#include <sstream>
#include <vector>

#include "account_manager.h"
#include "database_executor.h"
#include "log.hpp"
#include "offsets.h"
#include "rpc_helper.h"
#include "spy.h"
#include "spy_types.h"
#include "util.h"

namespace message
{

namespace OsSend = Offsets::Message::Send;

Sender &Sender::get_instance()
{
    static Sender instance;
    return instance;
}

Sender::Sender()
{
    func_new_chat_msg    = Spy::getFunction<New_t>(OsSend::INSTANCE);
    func_free_chat_msg   = Spy::getFunction<Free_t>(OsSend::FREE);
    func_send_msg_mgr    = Spy::getFunction<SendMsgMgr_t>(OsSend::MGR);
    func_send_text       = Spy::getFunction<SendText_t>(OsSend::TEXT);
    func_send_image      = Spy::getFunction<SendImage_t>(OsSend::IMAGE);
    func_get_app_mgr     = Spy::getFunction<GetAppMgr_t>(OsSend::APP_MGR);
    func_send_file       = Spy::getFunction<SendFile_t>(OsSend::FILE);
    func_new_mmreader    = Spy::getFunction<New_t>(OsSend::NEW_MM_READER);
    func_free_mmreader   = Spy::getFunction<Free_t>(OsSend::FREE_MM_READER);
    func_send_rich_text  = Spy::getFunction<SendRichText_t>(OsSend::RICH_TEXT);
    func_send_pat        = Spy::getFunction<SendPat_t>(OsSend::PAT);
    func_forward         = Spy::getFunction<Forward_t>(OsSend::FORWARD);
    func_get_emotion_mgr = Spy::getFunction<GetEmotionMgr_t>(OsSend::EMOTION_MGR);
    func_send_emotion    = Spy::getFunction<SendEmotion_t>(OsSend::EMOTION);
    func_send_xml        = Spy::getFunction<SendXml_t>(OsSend::XML);
    func_xml_buf_sign    = Spy::getFunction<XmlBufSign_t>(OsSend::XML_BUF_SIGN);
}

void Sender::send_text(const std::string &wxid, const std::string &msg, const std::string &at_wxids)
{
    util::WxStringHolder<std::string> holderMsg(msg);
    util::WxStringHolder<std::string> holderWxid(wxid);

    auto wxAtWxids   = util::parse_wxids(at_wxids).wxWxids;
    QWORD pWxAtWxids = wxAtWxids.empty() ? 0 : reinterpret_cast<QWORD>(&wxAtWxids);

    char buffer[1104] = { 0 };
    func_send_msg_mgr();
    func_send_text(reinterpret_cast<QWORD>(&buffer), &holderWxid.wx, &holderMsg.wx, pWxAtWxids, 1, 1, 0, 0);
    func_free_chat_msg(reinterpret_cast<QWORD>(&buffer));
}

void Sender::send_image(const std::string &wxid, const std::string &path)
{
    util::WxStringHolder<std::string> holderWxid(wxid);
    util::WxStringHolder<std::string> holderPath(path);

    char msg[1192]    = { 0 };
    char msgTmp[1192] = { 0 };
    QWORD *flag[10]   = { 0 };

    QWORD tmp1 = 1, tmp2 = 0, tmp3 = 0;
    QWORD pMsgTmp = func_new_chat_msg((QWORD)(&msgTmp));
    flag[0]       = reinterpret_cast<QWORD *>(tmp1);
    flag[1]       = reinterpret_cast<QWORD *>(pMsgTmp);
    flag[8]       = &tmp2;
    flag[9]       = &tmp3;

    QWORD pMsg    = func_new_chat_msg((QWORD)(&msg));
    QWORD sendMgr = func_send_msg_mgr();
    func_send_image(sendMgr, pMsg, &holderWxid.wx, &holderPath.wx, reinterpret_cast<QWORD>(&flag));

    func_free_chat_msg(pMsg);
    func_free_chat_msg(pMsgTmp);
}

void Sender::send_file(const std::string &wxid, const std::string &path)
{
    WxString *wxWxid = util::CreateWxString(wxid);
    WxString *wxPath = util::CreateWxString(path);
    if (!wxWxid || !wxPath) {
        util::FreeWxString(wxWxid);
        util::FreeWxString(wxPath);
        return;
    }

    char *chat_msg = reinterpret_cast<char *>(util::AllocFromHeap(0x460));
    if (!chat_msg) {
        util::FreeWxString(wxWxid);
        util::FreeWxString(wxPath);
        return;
    }

    QWORD *tmp1 = util::AllocBuffer<QWORD>(4);
    QWORD *tmp2 = util::AllocBuffer<QWORD>(4);
    QWORD *tmp3 = util::AllocBuffer<QWORD>(4);
    if (!tmp1 || !tmp2 || !tmp3) {
        func_free_chat_msg(reinterpret_cast<QWORD>(chat_msg));
        util::FreeBuffer(chat_msg);
        util::FreeBuffer(tmp1);
        util::FreeBuffer(tmp2);
        util::FreeBuffer(tmp3);
        util::FreeWxString(wxWxid);
        util::FreeWxString(wxPath);
        return;
    }

    QWORD app_mgr = func_get_app_mgr();
    func_send_file(app_mgr, chat_msg, wxWxid, wxPath, 1, tmp1, 0, tmp2, 0, tmp3, 0, 0xC);
    func_free_chat_msg(reinterpret_cast<QWORD>(chat_msg));

    util::FreeBuffer(chat_msg);
    util::FreeBuffer(tmp1);
    util::FreeBuffer(tmp2);
    util::FreeBuffer(tmp3);
    util::FreeWxString(wxWxid);
    util::FreeWxString(wxPath);
}

void Sender::send_xml(const std::string &receiver, const std::string &xml, const std::string &path, uint64_t type)
{
    char buff1[0x500] = { 0 };
    char buff2[0x500] = { 0 };
    char buff3[0x1C]  = { 0 };

    auto pBuff1 = func_new_chat_msg(reinterpret_cast<QWORD>(buff1));
    auto pBuff2 = func_new_chat_msg(reinterpret_cast<QWORD>(buff2));
    auto pBuff3 = reinterpret_cast<QWORD>(&buff3);

    QWORD array[4] = { 0 };

    auto sign = func_xml_buf_sign(pBuff2, reinterpret_cast<QWORD>(&array), 0x1);

    util::WxStringHolder<std::string> to(receiver);
    util::WxStringHolder<std::string> body(xml);
    util::WxStringHolder<std::string> thumb(path);
    util::WxStringHolder<std::string> from(account::get_self_wxid());

    func_send_xml(pBuff1, &from.wx, &to.wx, &body.wx, &thumb.wx, pBuff3, type, 0x4, sign, pBuff2);
    func_free_chat_msg(pBuff1);
    func_free_chat_msg(pBuff2);
}

void Sender::send_emotion(const std::string &wxid, const std::string &path)
{
    WxString *wxWxid = util::CreateWxString(wxid);
    WxString *wxPath = util::CreateWxString(path);
    QWORD *buff      = util::AllocBuffer<QWORD>(8);
    if (!wxWxid || !wxPath || !buff) {
        util::FreeWxString(wxWxid);
        util::FreeWxString(wxPath);
        util::FreeBuffer(buff);
        return;
    }

    QWORD mgr = func_get_emotion_mgr();
    func_send_emotion(mgr, wxPath, buff, wxWxid, 2, buff, 0, buff);
    util::FreeBuffer(buff);
    util::FreeWxString(wxWxid);
    util::FreeWxString(wxPath);
}

int Sender::send_rich_text(const RichText &rt)
{
    QWORD status = -1;

    char *buff          = util::AllocBuffer<char>(0x3F0);
    WxString *pReceiver = util::CreateWxString(rt.receiver);
    WxString *pTitle    = util::CreateWxString(rt.title);
    WxString *pUrl      = util::CreateWxString(rt.url);
    WxString *pThumburl = util::CreateWxString(rt.thumburl);
    WxString *pDigest   = util::CreateWxString(rt.digest);
    WxString *pAccount  = util::CreateWxString(rt.account);
    WxString *pName     = util::CreateWxString(rt.name);
    if (!buff || !pReceiver || !pTitle || !pUrl || !pThumburl || !pDigest || !pAccount || !pName) {
        util::FreeWxString(pReceiver);
        util::FreeWxString(pTitle);
        util::FreeWxString(pUrl);
        util::FreeWxString(pThumburl);
        util::FreeWxString(pDigest);
        util::FreeWxString(pAccount);
        util::FreeWxString(pName);
        util::FreeBuffer(buff);
        LOG_ERROR("Out of Memory...");
        return static_cast<int>(status);
    }

    func_new_mmreader(reinterpret_cast<QWORD>(buff));
    memcpy(buff + 0x8, pTitle, sizeof(WxString));
    memcpy(buff + 0x48, pUrl, sizeof(WxString));
    memcpy(buff + 0xB0, pThumburl, sizeof(WxString));
    memcpy(buff + 0xF0, pDigest, sizeof(WxString));
    memcpy(buff + 0x2C0, pAccount, sizeof(WxString));
    memcpy(buff + 0x2E0, pName, sizeof(WxString));

    status = func_send_rich_text(func_get_app_mgr(), pReceiver, buff);
    func_free_mmreader(reinterpret_cast<QWORD>(buff));

    // TODO: 验证是否有内存泄露
    // util::FreeWxString(pReceiver);
    // util::FreeWxString(pTitle);
    // util::FreeWxString(pUrl);
    // util::FreeWxString(pThumburl);
    // util::FreeWxString(pDigest);
    // util::FreeWxString(pAccount);
    // util::FreeWxString(pName);
    util::FreeBuffer(buff);

    return static_cast<int>(status);
}

int Sender::send_pat(const std::string &roomid, const std::string &wxid)
{
    QWORD status = -1;

    util::WxStringHolder<std::string> holderRoom(roomid);
    util::WxStringHolder<std::string> holderWxid(wxid);

    status = func_send_pat(&holderRoom.wx, &holderWxid.wx);

    return static_cast<int>(status);
}

int Sender::forward(QWORD msgid, const std::string &receiver)
{
    uint32_t dbIdx = 0;
    QWORD localId  = 0;

    if (db::get_local_id_and_dbidx(msgid, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", msgid);
        return -1;
    }

    LARGE_INTEGER l;
    l.HighPart = dbIdx;
    l.LowPart  = static_cast<DWORD>(localId);

    WxString *pReceiver = util::CreateWxString(receiver);

    return static_cast<int>(func_forward(pReceiver, l.QuadPart, 0x4, 0x0));
}

// RPC 方法
bool Sender::rpc_send_text(const TextMsg &text, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_SEND_TXT>(out, len, [&](Response &rsp) {
        if (text.msg == nullptr || text.receiver == nullptr || strlen(text.msg) == 0 || strlen(text.receiver) == 0) {
            LOG_ERROR("Empty message or receiver.");
            rsp.msg.status = -1;
        } else {
            send_text(text.receiver, text.msg, text.aters ? text.aters : "");
            rsp.msg.status = 0;
        }
    });
}

bool Sender::rpc_send_image(const PathMsg &file, uint8_t *out, size_t *len)
{
    std::string path(file.path);
    std::string receiver(file.receiver);
    return fill_response<Functions_FUNC_SEND_IMG>(out, len, [&](Response &rsp) {
        if (path.empty() || receiver.empty()) {
            LOG_ERROR("Empty path or receiver.");
            rsp.msg.status = -1;
        } else {
            send_image(receiver, path);
            rsp.msg.status = 0;
        }
    });
}

bool Sender::rpc_send_file(const PathMsg &file, uint8_t *out, size_t *len)
{
    std::string path(file.path);
    std::string receiver(file.receiver);
    return fill_response<Functions_FUNC_SEND_FILE>(out, len, [&](Response &rsp) {
        if (path.empty() || receiver.empty()) {
            LOG_ERROR("Empty path or receiver.");
            rsp.msg.status = -1;
        } else {
            send_file(receiver, path);
            rsp.msg.status = 0;
        }
    });
}

bool Sender::rpc_send_emotion(const PathMsg &file, uint8_t *out, size_t *len)
{
    std::string path(file.path);
    std::string receiver(file.receiver);
    return fill_response<Functions_FUNC_SEND_EMOTION>(out, len, [&](Response &rsp) {
        if (path.empty() || receiver.empty()) {
            LOG_ERROR("Empty path or receiver.");
            rsp.msg.status = -1;
        } else {
            send_emotion(receiver, path);
            rsp.msg.status = 0;
        }
    });
}

bool Sender::rpc_send_xml(const XmlMsg &xml, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_SEND_XML>(out, len, [&](Response &rsp) {
        if (xml.content == nullptr || xml.receiver == nullptr) {
            LOG_ERROR("Empty content or receiver.");
            rsp.msg.status = -1;
        } else {
            // send_xml(xml.receiver, xml.content, xml.path, xml.type);
            rsp.msg.status = -1;
        }
    });
}

bool Sender::rpc_send_rich_text(const RichText &rt, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_SEND_RICH_TXT>(out, len, [&](Response &rsp) {
        if (rt.receiver == nullptr) {
            LOG_ERROR("Empty receiver.");
            rsp.msg.status = -1;
        } else {
            rsp.msg.status = send_rich_text(rt);
        }
    });
}

bool Sender::rpc_send_pat(const PatMsg &pat, uint8_t *out, size_t *len)
{
    std::string wxid(pat.wxid);
    std::string roomid(pat.roomid);
    return fill_response<Functions_FUNC_SEND_PAT_MSG>(out, len, [&](Response &rsp) {
        if (roomid.empty() || wxid.empty()) {
            LOG_ERROR("Empty roomid or wxid.");
            rsp.msg.status = -1;
        } else {
            rsp.msg.status = send_pat(roomid, wxid);
        }
    });
}

bool Sender::rpc_forward(const ForwardMsg &fm, uint8_t *out, size_t *len)
{
    uint64_t msgid = fm.id;
    std::string receiver(fm.receiver);
    return fill_response<Functions_FUNC_FORWARD_MSG>(out, len, [&](Response &rsp) {
        if (receiver.empty()) {
            LOG_ERROR("Empty receiver.");
            rsp.msg.status = -1;
        } else {
            rsp.msg.status = forward(msgid, receiver);
        }
    });
}

}
