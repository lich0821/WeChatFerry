#include "message_sender.h"

#include <sstream>
#include <vector>

#include "database_executor.h"
#include "log.hpp"
#include "rpc_helper.h"
#include "spy_types.h"
#include "userinfo_manager.h"
#include "util.h"

extern QWORD g_WeChatWinDllAddr;

#define OS_NEW             0x1B5E140
#define OS_FREE            0x1B55850
#define OS_SEND_MSG_MGR    0x1B53FD0
#define OS_SEND_TEXT       0x22C6B60
#define OS_SEND_IMAGE      0x22BC2F0
#define OS_GET_APP_MSG_MGR 0x1B58F70
#define OS_SEND_FILE       0x20D0230
#define OS_RTM_NEW         0x1B5D690
#define OS_RTM_FREE        0x1B5CA60
#define OS_SEND_RICH_TEXT  0x20DA210
#define OS_SEND_PAT_MSG    0x2CAEC00
#define OS_FORWARD_MSG     0x22C60E0
#define OS_GET_EMOTION_MGR 0x1BCEF10
#define OS_SEND_EMOTION    0x21B52D5
#define OS_XML_BUFSIGN     0x24F0D70
#define OS_SEND_XML        0x20CF360

SendMsgManager &SendMsgManager::get_instance()
{
    static SendMsgManager instance;
    return instance;
}

SendMsgManager::SendMsgManager()
{
    func_new            = reinterpret_cast<New_t>(g_WeChatWinDllAddr + OS_NEW);
    func_free           = reinterpret_cast<Free_t>(g_WeChatWinDllAddr + OS_FREE);
    func_send_msg_mgr   = reinterpret_cast<SendMsgMgr_t>(g_WeChatWinDllAddr + OS_SEND_MSG_MGR);
    func_send_text      = reinterpret_cast<SendText_t>(g_WeChatWinDllAddr + OS_SEND_TEXT);
    func_send_image     = reinterpret_cast<SendImage_t>(g_WeChatWinDllAddr + OS_SEND_IMAGE);
    func_send_file      = reinterpret_cast<SendFile_t>(g_WeChatWinDllAddr + OS_SEND_FILE);
    func_send_rich_text = reinterpret_cast<SendRichText_t>(g_WeChatWinDllAddr + OS_SEND_RICH_TEXT);
    func_send_pat       = reinterpret_cast<SendPat_t>(g_WeChatWinDllAddr + OS_SEND_PAT_MSG);
    func_forward        = reinterpret_cast<Forward_t>(g_WeChatWinDllAddr + OS_FORWARD_MSG);
    func_send_emotion   = reinterpret_cast<SendEmotion_t>(g_WeChatWinDllAddr + OS_SEND_EMOTION);
    func_send_xml       = reinterpret_cast<SendXml_t>(g_WeChatWinDllAddr + OS_SEND_XML);
}

std::unique_ptr<WxString> SendMsgManager::new_wx_string(const char *str)
{
    return new_wx_string(str ? std::string(str) : std::string());
}
std::unique_ptr<WxString> SendMsgManager::new_wx_string(const std::string &str)
{
    return std::make_unique<WxString>(util::s2w(str));
}

std::vector<WxString> SendMsgManager::parse_wxids(const string &wxids)
{
    vector<WxString> wx_members;
    wstringstream wss(util::s2w(wxids));
    wstring wstr;
    while (getline(wss, wstr, L',')) {
        wx_members.emplace_back(wstr);
    }
    return wx_members;
}

void SendMsgManager::send_text(const std::string &wxid, const std::string &msg, const std::string &at_wxids)
{
    auto wxWxid = new_wx_string(wxid);
    auto wxMsg  = new_wx_string(msg);

    std::vector<WxString> wx_at_wxids;
    if (!at_wxids.empty()) {
        wx_at_wxids = parse_wxids(at_wxids);
    } else {
        wx_at_wxids.emplace_back();
    }

    QWORD wx_ater_list = reinterpret_cast<QWORD>(&(wx_at_wxids.front()));

    char buffer[0x460] = { 0 };
    func_send_msg_mgr();
    func_send_text(reinterpret_cast<QWORD>(&buffer), reinterpret_cast<QWORD>(wxWxid.get()),
                   reinterpret_cast<QWORD>(wxMsg.get()), wx_ater_list, 1, 1, 0, 0);
    func_free(reinterpret_cast<QWORD>(&buffer));
}

void SendMsgManager::send_image(const std::string &wxid, const std::string &path)
{
    auto wxWxid = new_wx_string(wxid);
    auto wxPath = new_wx_string(path);

    char msg[0x460]    = { 0 };
    char msgTmp[0x460] = { 0 };
    QWORD *flag[10]    = { nullptr };

    QWORD tmp1 = 0, tmp2 = 0;
    QWORD pMsgTmp = func_new(reinterpret_cast<QWORD>(&msgTmp));
    flag[8]       = &tmp1;
    flag[9]       = &tmp2;
    flag[1]       = reinterpret_cast<QWORD *>(pMsgTmp);

    QWORD pMsg    = func_new(reinterpret_cast<QWORD>(&msg));
    QWORD sendMgr = func_send_msg_mgr();

    func_send_image(sendMgr, pMsg, reinterpret_cast<QWORD>(wxWxid.get()), reinterpret_cast<QWORD>(wxPath.get()),
                    reinterpret_cast<QWORD>(&flag));

    func_free(pMsg);
    func_free(pMsgTmp);
}

void SendMsgManager::send_file(const std::string &wxid, const std::string &path)
{
    auto wxWxid = new_wx_string(wxid);
    auto wxPath = new_wx_string(path);

    char msg[0x460] = { 0 };
    QWORD tmp1[4]   = { 0 };
    QWORD tmp2[4]   = { 0 };
    QWORD tmp3[4]   = { 0 };

    QWORD pMsg   = func_new(reinterpret_cast<QWORD>(&msg));
    QWORD appMgr = func_get_app_mgr();

    func_send_file(appMgr, pMsg, reinterpret_cast<QWORD>(wxWxid.get()), reinterpret_cast<QWORD>(wxPath.get()), 1, tmp1,
                   0, tmp2, 0, tmp3, 0, 0);

    func_free(pMsg);
}

void SendMsgManager::send_xml(const std::string &receiver, const std::string &xml, const std::string &path,
                              uint64_t type)
{
    std::unique_ptr<char[]> buff(new char[0x500]());
    std::unique_ptr<char[]> buff2(new char[0x500]());
    char nullBuf[0x1C] = { 0 };

    func_new(reinterpret_cast<QWORD>(buff.get()));
    func_new(reinterpret_cast<QWORD>(buff2.get()));

    QWORD sbuf[4] = { 0, 0, 0, 0 };
    QWORD sign    = func_xml_buf_sign(reinterpret_cast<QWORD>(buff2.get()), reinterpret_cast<QWORD>(sbuf), 0x1);

    auto wxReceiver = new_wx_string(receiver);
    auto wxXml      = new_wx_string(xml);
    auto wxPath     = new_wx_string(path);
    auto wxSender   = new_wx_string(user_info::get_self_wxid());

    func_send_xml(reinterpret_cast<QWORD>(buff.get()), reinterpret_cast<QWORD>(wxSender.get()),
                  reinterpret_cast<QWORD>(wxReceiver.get()), reinterpret_cast<QWORD>(wxXml.get()),
                  reinterpret_cast<QWORD>(wxPath.get()), reinterpret_cast<QWORD>(nullBuf), type, 0x4, sign,
                  reinterpret_cast<QWORD>(buff2.get()));

    func_free(reinterpret_cast<QWORD>(buff.get()));
    func_free(reinterpret_cast<QWORD>(buff2.get()));
}

void SendMsgManager::send_emotion(const std::string &wxid, const std::string &path)
{
    auto wxWxid = new_wx_string(wxid);
    auto wxPath = new_wx_string(path);

    std::unique_ptr<QWORD[]> buff(new QWORD[8]()); // 0x20 bytes = 8 * QWORD

    if (!buff) {
        LOG_ERROR("Out of Memory...");
        return;
    }

    QWORD mgr = func_get_emotion_mgr();
    func_send_emotion(mgr, reinterpret_cast<QWORD>(wxPath.get()), reinterpret_cast<QWORD>(buff.get()),
                      reinterpret_cast<QWORD>(wxWxid.get()), 2, reinterpret_cast<QWORD>(buff.get()), 0,
                      reinterpret_cast<QWORD>(buff.get()));
}

int SendMsgManager::send_rich_text(const RichText &rt)
{
#define SRTM_SIZE 0x3F0
    QWORD status = -1;

    char *buff = static_cast<char *>(HeapAlloc(GetProcessHeap(), 0, SRTM_SIZE));
    if (!buff) {
        LOG_ERROR("Out of Memory...");
        return -1;
    }

    memset(buff, 0, SRTM_SIZE);
    func_new(reinterpret_cast<QWORD>(buff));

    auto pReceiver = new_wx_string(rt.receiver);
    auto pTitle    = new_wx_string(rt.title);
    auto pUrl      = new_wx_string(rt.url);
    auto pThumburl = new_wx_string(rt.thumburl);
    auto pDigest   = new_wx_string(rt.digest);
    auto pAccount  = new_wx_string(rt.account);
    auto pName     = new_wx_string(rt.name);

    memcpy(buff + 0x8, pTitle.get(), sizeof(WxString));
    memcpy(buff + 0x48, pUrl.get(), sizeof(WxString));
    memcpy(buff + 0xB0, pThumburl.get(), sizeof(WxString));
    memcpy(buff + 0xF0, pDigest.get(), sizeof(WxString));
    memcpy(buff + 0x2C0, pAccount.get(), sizeof(WxString));
    memcpy(buff + 0x2E0, pName.get(), sizeof(WxString));

    QWORD mgr = func_get_app_mgr();
    status    = func_send_rich_text(mgr, reinterpret_cast<QWORD>(pReceiver.get()), reinterpret_cast<QWORD>(buff));
    func_free(reinterpret_cast<QWORD>(buff));

    return static_cast<int>(status);
}

int SendMsgManager::send_pat(const std::string &roomid, const std::string &wxid)
{
    QWORD status = -1;

    auto wxRoomid = new_wx_string(roomid);
    auto wxWxid   = new_wx_string(wxid);

    status = func_send_pat(reinterpret_cast<QWORD>(wxRoomid.get()), reinterpret_cast<QWORD>(wxWxid.get()));

    return static_cast<int>(status);
}

int SendMsgManager::forward(QWORD msgid, const std::string &receiver)
{
    uint32_t dbIdx = 0;
    QWORD localId  = 0;

    if (exec_sql::get_local_id_and_dbidx(msgid, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", msgid);
        return -1;
    }

    LARGE_INTEGER l;
    l.HighPart      = dbIdx;
    l.LowPart       = static_cast<DWORD>(localId);
    auto wxReceiver = new_wx_string(receiver);

    return static_cast<int>(func_forward(reinterpret_cast<QWORD>(wxReceiver.get()), l.QuadPart, 0x4, 0x0));
}

// RPC 方法
bool SendMsgManager::rpc_send_text(const TextMsg &text, uint8_t *out, size_t *len)
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

bool SendMsgManager::rpc_send_image(const std::string &path, const std::string &receiver, uint8_t *out, size_t *len)
{
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

bool SendMsgManager::rpc_send_file(const std::string &path, const std::string &receiver, uint8_t *out, size_t *len)
{
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

bool SendMsgManager::rpc_send_emotion(const std::string &path, const std::string &receiver, uint8_t *out, size_t *len)
{
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

bool SendMsgManager::rpc_send_xml(const XmlMsg &xml, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_SEND_XML>(out, len, [&](Response &rsp) {
        if (xml.content == nullptr || xml.receiver == nullptr) {
            LOG_ERROR("Empty content or receiver.");
            rsp.msg.status = -1;
        } else {
            send_xml(xml.receiver, xml.content, xml.path, xml.type);
            rsp.msg.status = 0;
        }
    });
}

bool SendMsgManager::rpc_send_rich_text(const RichText &rt, uint8_t *out, size_t *len)
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

bool SendMsgManager::rpc_send_pat(const std::string &roomid, const std::string &wxid, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_SEND_PAT_MSG>(out, len, [&](Response &rsp) {
        if (roomid.empty() || wxid.empty()) {
            LOG_ERROR("Empty roomid or wxid.");
            rsp.msg.status = -1;
        } else {
            rsp.msg.status = send_pat(roomid, wxid);
        }
    });
}

bool SendMsgManager::rpc_forward(uint64_t msgid, const std::string &receiver, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_FORWARD_MSG>(out, len, [&](Response &rsp) {
        if (receiver.empty()) {
            LOG_ERROR("Empty receiver.");
            rsp.msg.status = -1;
        } else {
            rsp.msg.status = forward(msgid, receiver);
        }
    });
}
