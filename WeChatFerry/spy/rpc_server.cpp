#pragma warning(disable : 4251)

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <thread>

#include <magic_enum/magic_enum.hpp>
#include <nng/nng.h>
#include <nng/protocol/pair1/pair.h>
#include <nng/supplemental/util/platform.h>

#include "wcf.pb.h"

#include "chatroom_mgmt.h"
#include "contact_mgmt.h"
#include "exec_sql.h"
#include "funcs.h"
#include "log.hpp"
#include "pb_types.h"
#include "pb_util.h"
#include "receive_msg.h"
#include "rpc_server.h"
#include "send_msg.h"
#include "spy.h"
#include "spy_types.h"
#include "user_info.h"
#include "util.h"

namespace fs = std::filesystem;

constexpr size_t DEFAULT_BUF_SIZE = 16 * 1024 * 1024;

static int cmdPort        = 0;
static bool isRpcRunning  = false;
static HANDLE cmdThread   = NULL;
static HANDLE msgThread   = NULL;
static nng_socket cmdSock = NNG_SOCKET_INITIALIZER; // TODO: 断开检测
static nng_socket msgSock = NNG_SOCKET_INITIALIZER; // TODO: 断开检测

auto &msgHandler = MessageHandler::getInstance();

static std::string BuildUrl(int port) { return "tcp://0.0.0.0:" + std::to_string(port); }

template <Functions funcType, typename AssignFunc>
static bool FillResponse(int which_msg, uint8_t *out, size_t *len, AssignFunc assign)
{
    Response rsp  = Response_init_default;
    rsp.func      = funcType;
    rsp.which_msg = which_msg;

    assign(rsp);

    pb_ostream_t stream = pb_ostream_from_buffer(out, *len);
    if (!pb_encode(&stream, Response_fields, &rsp)) {
        LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(&stream));
        return false;
    }
    *len = stream.bytes_written;
    return true;
}

static bool func_is_login(uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_IS_LOGIN>(Response_status_tag, out, len,
                                                 [](Response &rsp) { rsp.msg.status = IsLogin(); });
}

static bool func_get_self_wxid(uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_GET_SELF_WXID>(Response_str_tag, out, len, [](Response &rsp) {
        std::string wxid = user_info::get_self_wxid();
        rsp.msg.str      = wxid.empty() ? nullptr : (char *)wxid.c_str();
    });
}

static bool func_get_user_info(uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_GET_USER_INFO>(Response_ui_tag, out, len, [](Response &rsp) {
        UserInfo_t ui     = user_info::get_user_info();
        rsp.msg.ui.wxid   = (char *)ui.wxid.c_str();
        rsp.msg.ui.name   = (char *)ui.name.c_str();
        rsp.msg.ui.mobile = (char *)ui.mobile.c_str();
        rsp.msg.ui.home   = (char *)ui.home.c_str();
    });
}

static bool func_get_msg_types(uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_GET_MSG_TYPES>(Response_types_tag, out, len, [](Response &rsp) {
        static MsgTypes_t types          = msgHandler.GetMsgTypes();
        rsp.msg.types.types.funcs.encode = encode_types;
        rsp.msg.types.types.arg          = &types;
    });
}

static bool func_get_contacts(uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_GET_CONTACTS>(Response_contacts_tag, out, len, [](Response &rsp) {
        static std::vector<RpcContact_t> contacts = contact_mgmt::get_contacts();
        rsp.msg.contacts.contacts.funcs.encode    = encode_contacts;
        rsp.msg.contacts.contacts.arg             = &contacts;
    });
}

static bool func_get_db_names(uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_GET_DB_NAMES>(Response_dbs_tag, out, len, [](Response &rsp) {
        static DbNames_t dbnames       = GetDbNames();
        rsp.msg.dbs.names.funcs.encode = encode_dbnames;
        rsp.msg.dbs.names.arg          = &dbnames;
    });
}

static bool func_get_db_tables(char *db, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_GET_DB_TABLES>(Response_tables_tag, out, len, [db](Response &rsp) {
        static DbTables_t tables           = GetDbTables(db);
        rsp.msg.tables.tables.funcs.encode = encode_tables;
        rsp.msg.tables.tables.arg          = &tables;
    });
}

static bool func_get_audio_msg(uint64_t id, char *dir, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_GET_AUDIO_MSG>(Response_str_tag, out, len, [id, dir](Response &rsp) {
        std::string path = (dir == nullptr) ? "" : GetAudio(id, dir);
        rsp.msg.str      = path.empty() ? nullptr : (char *)path.c_str();
    });
}

static bool func_send_txt(TextMsg txt, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_SEND_TXT>(Response_status_tag, out, len, [txt](Response &rsp) {
        if ((txt.msg == NULL) || (txt.receiver == NULL)) {
            LOG_ERROR("Empty message or receiver.");
            rsp.msg.status = -1;
        } else {
            std::string msg(txt.msg);
            std::string receiver(txt.receiver);
            std::string aters(txt.aters ? txt.aters : "");
            SendTextMessage(receiver, msg, aters);
            rsp.msg.status = 0;
        }
    });
}

static bool func_send_img(char *path, char *receiver, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_SEND_IMG>(Response_status_tag, out, len, [path, receiver](Response &rsp) {
        if ((path == NULL) || (receiver == NULL)) {
            LOG_ERROR("Empty path or receiver.");
            rsp.msg.status = -1;
        } else if (!fs::exists(String2Wstring(path))) {
            LOG_ERROR("Path does not exist: {}", path);
            rsp.msg.status = -2;
        } else {
            SendImageMessage(receiver, path);
            rsp.msg.status = 0;
        }
    });
}

static bool func_send_file(char *path, char *receiver, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_SEND_FILE>(Response_status_tag, out, len, [path, receiver](Response &rsp) {
        if ((path == nullptr) || (receiver == nullptr)) {
            LOG_ERROR("Empty path or receiver.");
            rsp.msg.status = -1;
        } else if (!fs::exists(String2Wstring(path))) {
            LOG_ERROR("Path does not exist: {}", path);
            rsp.msg.status = -2;
        } else {
            SendFileMessage(receiver, path);
            rsp.msg.status = 0;
        }
    });
}

static bool func_send_emotion(char *path, char *receiver, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_SEND_EMOTION>(Response_status_tag, out, len, [path, receiver](Response &rsp) {
        if ((path == nullptr) || (receiver == nullptr)) {
            LOG_ERROR("Empty path or receiver.");
            rsp.msg.status = -1;
        } else {
            SendEmotionMessage(receiver, path);
            rsp.msg.status = 0;
        }
    });
}

static bool func_send_xml(XmlMsg xml, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_SEND_XML>(Response_status_tag, out, len, [xml](Response &rsp) {
        if ((xml.content == nullptr) || (xml.receiver == nullptr)) {
            LOG_ERROR("Empty content or receiver.");
            rsp.msg.status = -1;
        } else {
            std::string receiver(xml.receiver);
            std::string content(xml.content);
            std::string path(xml.path ? xml.path : "");
            uint64_t type = static_cast<uint64_t>(xml.type);
            SendXmlMessage(receiver, content, path, type);
            rsp.msg.status = 0;
        }
    });
}

static bool func_send_rich_txt(RichText rt, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_SEND_RICH_TXT>(Response_status_tag, out, len, [rt](Response &rsp) {
        if (rt.receiver == nullptr) {
            LOG_ERROR("Empty receiver.");
            rsp.msg.status = -1;
        } else {
            RichText_t rtt;
            rtt.account  = std::string(rt.account ? rt.account : "");
            rtt.digest   = std::string(rt.digest ? rt.digest : "");
            rtt.name     = std::string(rt.name ? rt.name : "");
            rtt.receiver = std::string(rt.receiver ? rt.receiver : "");
            rtt.thumburl = std::string(rt.thumburl ? rt.thumburl : "");
            rtt.title    = std::string(rt.title ? rt.title : "");
            rtt.url      = std::string(rt.url ? rt.url : "");

            rsp.msg.status = SendRichTextMessage(rtt);
        }
    });
}

static bool func_send_pat_msg(char *roomid, char *wxid, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_SEND_PAT_MSG>(Response_status_tag, out, len, [roomid, wxid](Response &rsp) {
        if ((roomid == nullptr) || (wxid == nullptr)) {
            LOG_ERROR("Empty roomid or wxid.");
            rsp.msg.status = -1;
        } else {
            rsp.msg.status = SendPatMessage(roomid, wxid);
        }
    });
}

static bool func_forward_msg(uint64_t id, char *receiver, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_FORWARD_MSG>(Response_status_tag, out, len, [id, receiver](Response &rsp) {
        if (receiver == nullptr) {
            LOG_ERROR("Empty receiver.");
            rsp.msg.status = -1;
        } else {
            rsp.msg.status = ForwardMessage(id, receiver);
        }
    });
}

static void PushMessage()
{
    int rv;
    Response rsp  = Response_init_default;
    rsp.func      = Functions_FUNC_ENABLE_RECV_TXT;
    rsp.which_msg = Response_wxmsg_tag;
    std::vector<uint8_t> msgBuffer(DEFAULT_BUF_SIZE);

    pb_ostream_t stream = pb_ostream_from_buffer(msgBuffer.data(), msgBuffer.size());

    std::string url = BuildUrl(cmdPort + 1);
    if ((rv = nng_pair1_open(&msgSock)) != 0) {
        LOG_ERROR("nng_pair0_open error {}", nng_strerror(rv));
        return;
    }

    if ((rv = nng_listen(msgSock, url.c_str(), NULL, 0)) != 0) {
        LOG_ERROR("nng_listen error {}", nng_strerror(rv));
        return;
    }

    LOG_INFO("MSG Server listening on {}", url.c_str());
    if ((rv = nng_setopt_ms(msgSock, NNG_OPT_SENDTIMEO, 5000)) != 0) {
        LOG_ERROR("nng_setopt_ms: {}", nng_strerror(rv));
        return;
    }

    while (msgHandler.isMessageListening()) {
        std::unique_lock<std::mutex> lock(msgHandler.getMutex());
        std::optional<WxMsg_t> msgOpt;
        auto hasMessage = [&]() {
            msgOpt = msgHandler.popMessage();
            return msgOpt.has_value();
        };

        if (msgHandler.getConditionVariable().wait_for(lock, std::chrono::milliseconds(1000), hasMessage)) {
            WxMsg_t wxmsg          = std::move(msgOpt.value());
            rsp.msg.wxmsg.id       = wxmsg.id;
            rsp.msg.wxmsg.is_self  = wxmsg.is_self;
            rsp.msg.wxmsg.is_group = wxmsg.is_group;
            rsp.msg.wxmsg.type     = wxmsg.type;
            rsp.msg.wxmsg.ts       = wxmsg.ts;
            rsp.msg.wxmsg.roomid   = (char *)wxmsg.roomid.c_str();
            rsp.msg.wxmsg.content  = (char *)wxmsg.content.c_str();
            rsp.msg.wxmsg.sender   = (char *)wxmsg.sender.c_str();
            rsp.msg.wxmsg.sign     = (char *)wxmsg.sign.c_str();
            rsp.msg.wxmsg.thumb    = (char *)wxmsg.thumb.c_str();
            rsp.msg.wxmsg.extra    = (char *)wxmsg.extra.c_str();
            rsp.msg.wxmsg.xml      = (char *)wxmsg.xml.c_str();

            LOG_DEBUG("Push msg: {}", wxmsg.content);
            pb_ostream_t stream = pb_ostream_from_buffer(msgBuffer.data(), msgBuffer.size());
            if (!pb_encode(&stream, Response_fields, &rsp)) {
                LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(&stream));
                continue;
            }

            rv = nng_send(msgSock, msgBuffer.data(), stream.bytes_written, 0);
            if (rv != 0) {
                LOG_ERROR("msgSock-nng_send: {}", nng_strerror(rv));
            }
            LOG_DEBUG("Send data length {}", stream.bytes_written);
        }
    }
}

static bool func_enable_recv_txt(bool pyq, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_ENABLE_RECV_TXT>(Response_status_tag, out, len, [pyq](Response &rsp) {
        rsp.msg.status = msgHandler.ListenMsg();
        if (rsp.msg.status == 0) {
            if (pyq) {
                msgHandler.ListenPyq();
            }
            msgThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)PushMessage, nullptr, 0, nullptr);
            if (msgThread == nullptr) {
                rsp.msg.status = GetLastError();
                LOG_ERROR("func_enable_recv_txt failed: {}", rsp.msg.status);
            }
        }
    });
}

static bool func_disable_recv_txt(uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_DISABLE_RECV_TXT>(Response_status_tag, out, len, [](Response &rsp) {
        rsp.msg.status = msgHandler.UnListenMsg();
        if (rsp.msg.status == 0) {
            msgHandler.UnListenPyq();
            if (msgThread != nullptr) {
                TerminateThread(msgThread, 0);
                msgThread = nullptr;
            }
        }
    });
}

static bool func_exec_db_query(char *db, char *sql, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_EXEC_DB_QUERY>(Response_rows_tag, out, len, [db, sql](Response &rsp) {
        static DbRows_t rows;
        if ((db == nullptr) || (sql == nullptr)) {
            LOG_ERROR("Empty db or sql.");
        } else {
            rows = ExecDbQuery(db, sql);
        }
        rsp.msg.rows.rows.arg          = &rows;
        rsp.msg.rows.rows.funcs.encode = encode_rows;
    });
}

static bool func_refresh_pyq(uint64_t id, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_REFRESH_PYQ>(Response_status_tag, out, len,
                                                    [id](Response &rsp) { rsp.msg.status = RefreshPyq(id); });
}

static bool func_download_attach(AttachMsg att, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_DOWNLOAD_ATTACH>(Response_status_tag, out, len, [att](Response &rsp) {
        std::string thumb = att.thumb ? att.thumb : "";
        std::string extra = att.extra ? att.extra : "";
        rsp.msg.status    = DownloadAttach(att.id, thumb, extra);
    });
}

static bool func_revoke_msg(uint64_t id, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_REVOKE_MSG>(Response_status_tag, out, len,
                                                   [id](Response &rsp) { rsp.msg.status = RevokeMsg(id); });
}

static bool func_refresh_qrcode(uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_REFRESH_QRCODE>(Response_str_tag, out, len, [](Response &rsp) {
        std::string url = GetLoginUrl();
        rsp.msg.str     = url.empty() ? nullptr : (char *)url.c_str();
    });
}

static bool func_receive_transfer(char *wxid, char *tfid, char *taid, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_RECV_TRANSFER>(Response_status_tag, out, len, [wxid, tfid, taid](Response &rsp) {
        if ((wxid == nullptr) || (tfid == nullptr) || (taid == nullptr)) {
            LOG_ERROR("Empty wxid, tfid, or taid.");
            rsp.msg.status = -1;
        } else {
            rsp.msg.status = ReceiveTransfer(wxid, tfid, taid);
        }
    });
}

static bool func_accept_friend(char *v3, char *v4, int32_t scene, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_ACCEPT_FRIEND>(Response_status_tag, out, len, [v3, v4, scene](Response &rsp) {
        if ((v3 == nullptr) || (v4 == nullptr)) {
            LOG_ERROR("Empty V3 or V4.");
            rsp.msg.status = -1;
        } else {
            rsp.msg.status = contact_mgmt::accept_friend(v3, v4, scene);
        }
    });
}

static bool func_get_contact_info(std::string wxid, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_GET_CONTACT_INFO>(Response_contacts_tag, out, len, [wxid](Response &rsp) {
        std::vector<RpcContact_t> contacts     = contact_mgmt::get_contact_by_wxid(wxid);
        rsp.msg.contacts.contacts.funcs.encode = encode_contacts;
        rsp.msg.contacts.contacts.arg          = &contacts;
    });
}

static bool func_decrypt_image(DecPath dec, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_DECRYPT_IMAGE>(Response_str_tag, out, len, [dec](Response &rsp) {
        std::string path;
        if ((dec.src == nullptr) || (dec.dst == nullptr)) {
            LOG_ERROR("Empty src or dst.");
        } else {
            path = DecryptImage(dec.src, dec.dst);
        }
        rsp.msg.str = path.empty() ? nullptr : (char *)path.c_str();
    });
}

static bool func_exec_ocr(char *path, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_EXEC_OCR>(Response_ocr_tag, out, len, [path](Response &rsp) {
        OcrResult_t ret = { -1, "" };
        if (path == nullptr) {
            LOG_ERROR("Empty path.");
        } else {
            ret = GetOcrResult(path);
        }
        rsp.msg.ocr.status = ret.status;
        rsp.msg.ocr.result = ret.result.empty() ? nullptr : (char *)ret.result.c_str();
    });
}

static bool func_add_room_members(char *roomid, char *wxids, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_ADD_ROOM_MEMBERS>(Response_status_tag, out, len, [roomid, wxids](Response &rsp) {
        if ((roomid == nullptr) || (wxids == nullptr)) {
            LOG_ERROR("Empty roomid or wxids.");
            rsp.msg.status = -1;
        } else {
            rsp.msg.status = AddChatroomMember(roomid, wxids);
        }
    });
}

static bool func_del_room_members(char *roomid, char *wxids, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_DEL_ROOM_MEMBERS>(Response_status_tag, out, len, [roomid, wxids](Response &rsp) {
        if ((roomid == nullptr) || (wxids == nullptr)) {
            LOG_ERROR("Empty roomid or wxids.");
            rsp.msg.status = -1;
        } else {
            rsp.msg.status = DelChatroomMember(roomid, wxids);
        }
    });
}

static bool func_invite_room_members(char *roomid, char *wxids, uint8_t *out, size_t *len)
{
    return FillResponse<Functions_FUNC_INV_ROOM_MEMBERS>(Response_status_tag, out, len, [roomid, wxids](Response &rsp) {
        if ((roomid == nullptr) || (wxids == nullptr)) {
            LOG_ERROR("Empty roomid or wxids.");
            rsp.msg.status = -1;
        } else {
            rsp.msg.status = InviteChatroomMember(roomid, wxids);
        }
    });
}

static bool dispatcher(uint8_t *in, size_t in_len, uint8_t *out, size_t *out_len)
{
    bool ret            = false;
    Request req         = Request_init_default;
    pb_istream_t stream = pb_istream_from_buffer(in, in_len);
    if (!pb_decode(&stream, Request_fields, &req)) {
        LOG_ERROR("Decoding failed: {}", PB_GET_ERROR(&stream));
        pb_release(Request_fields, &req);
        return false;
    }

    LOG_DEBUG("{:#04x}[{}] length: {}", (uint8_t)req.func, magic_enum::enum_name(req.func), in_len);

    switch (req.func) {
        case Functions_FUNC_IS_LOGIN: {
            ret = func_is_login(out, out_len);
            break;
        }
        case Functions_FUNC_GET_SELF_WXID: {
            ret = func_get_self_wxid(out, out_len);
            break;
        }
        case Functions_FUNC_GET_USER_INFO: {
            ret = func_get_user_info(out, out_len);
            break;
        }
        case Functions_FUNC_GET_MSG_TYPES: {
            ret = func_get_msg_types(out, out_len);
            break;
        }
        case Functions_FUNC_GET_CONTACTS: {
            ret = func_get_contacts(out, out_len);
            break;
        }
        case Functions_FUNC_GET_DB_NAMES: {
            ret = func_get_db_names(out, out_len);
            break;
        }
        case Functions_FUNC_GET_DB_TABLES: {
            ret = func_get_db_tables(req.msg.str, out, out_len);
            break;
        }
        case Functions_FUNC_GET_AUDIO_MSG: {
            ret = func_get_audio_msg(req.msg.am.id, req.msg.am.dir, out, out_len);
            break;
        }
        case Functions_FUNC_SEND_TXT: {
            ret = func_send_txt(req.msg.txt, out, out_len);
            break;
        }
        case Functions_FUNC_SEND_IMG: {
            ret = func_send_img(req.msg.file.path, req.msg.file.receiver, out, out_len);
            break;
        }
        case Functions_FUNC_SEND_FILE: {
            ret = func_send_file(req.msg.file.path, req.msg.file.receiver, out, out_len);
            break;
        }
        case Functions_FUNC_SEND_RICH_TXT: {
            ret = func_send_rich_txt(req.msg.rt, out, out_len);
            break;
        }
        case Functions_FUNC_SEND_PAT_MSG: {
            ret = func_send_pat_msg(req.msg.pm.roomid, req.msg.pm.wxid, out, out_len);
            break;
        }
        case Functions_FUNC_FORWARD_MSG: {
            ret = func_forward_msg(req.msg.fm.id, req.msg.fm.receiver, out, out_len);
            break;
        }
        case Functions_FUNC_SEND_EMOTION: {
            ret = func_send_emotion(req.msg.file.path, req.msg.file.receiver, out, out_len);
            break;
        }
#if 0
        case Functions_FUNC_SEND_XML: {
            ret = func_send_xml(req.msg.xml, out, out_len);
            break;
        }
#endif
        case Functions_FUNC_ENABLE_RECV_TXT: {
            ret = func_enable_recv_txt(req.msg.flag, out, out_len);
            break;
        }
        case Functions_FUNC_DISABLE_RECV_TXT: {
            ret = func_disable_recv_txt(out, out_len);
            break;
        }
        case Functions_FUNC_EXEC_DB_QUERY: {
            ret = func_exec_db_query(req.msg.query.db, req.msg.query.sql, out, out_len);
            break;
        }
        case Functions_FUNC_REFRESH_PYQ: {
            ret = func_refresh_pyq(req.msg.ui64, out, out_len);
            break;
        }
        case Functions_FUNC_DOWNLOAD_ATTACH: {
            ret = func_download_attach(req.msg.att, out, out_len);
            break;
        }
        case Functions_FUNC_RECV_TRANSFER: {
            ret = func_receive_transfer(req.msg.tf.wxid, req.msg.tf.tfid, req.msg.tf.taid, out, out_len);
            break;
        }
        case Functions_FUNC_REVOKE_MSG: {
            ret = func_revoke_msg(req.msg.ui64, out, out_len);
            break;
        }
        case Functions_FUNC_REFRESH_QRCODE: {
            ret = func_refresh_qrcode(out, out_len);
            break;
        }
#if 0
        case Functions_FUNC_ACCEPT_FRIEND: {
            ret = func_accept_friend(req.msg.v.v3, req.msg.v.v4, req.msg.v.scene, out, out_len);
            break;
        }
        case Functions_FUNC_GET_CONTACT_INFO: {
            ret = func_get_contact_info(req.msg.str, out, out_len);
            break;
        }
#endif
        case Functions_FUNC_DECRYPT_IMAGE: {
            ret = func_decrypt_image(req.msg.dec, out, out_len);
            break;
        }
        case Functions_FUNC_EXEC_OCR: {
            ret = func_exec_ocr(req.msg.str, out, out_len);
            break;
        }
        case Functions_FUNC_ADD_ROOM_MEMBERS: {
            ret = func_add_room_members(req.msg.m.roomid, req.msg.m.wxids, out, out_len);
            break;
        }
        case Functions_FUNC_DEL_ROOM_MEMBERS: {
            ret = func_del_room_members(req.msg.m.roomid, req.msg.m.wxids, out, out_len);
            break;
        }
        case Functions_FUNC_INV_ROOM_MEMBERS: {
            ret = func_invite_room_members(req.msg.m.roomid, req.msg.m.wxids, out, out_len);
            break;
        }
        default: {
            LOG_ERROR("[UNKNOW FUNCTION]");
            break;
        }
    }

    pb_release(Request_fields, &req);
    return ret;
}

static int RunRpcServer()
{
    int rv          = 0;
    std::string url = BuildUrl(cmdPort);
    if ((rv = nng_pair1_open(&cmdSock)) != 0) {
        LOG_ERROR("nng_pair0_open error {}", nng_strerror(rv));
        return rv;
    }

    if ((rv = nng_listen(cmdSock, url.c_str(), NULL, 0)) != 0) {
        LOG_ERROR("nng_listen error {}", nng_strerror(rv));
        return rv;
    }

    LOG_INFO("CMD Server listening on {}", url.c_str());
    if ((rv = nng_setopt_ms(cmdSock, NNG_OPT_SENDTIMEO, 1000)) != 0) {
        LOG_ERROR("nng_setopt_ms error: {}", nng_strerror(rv));
        return rv;
    }

    std::vector<uint8_t> cmdBuffer(DEFAULT_BUF_SIZE);
    isRpcRunning = true;
    while (isRpcRunning) {
        uint8_t *in = NULL;
        size_t in_len, out_len = cmdBuffer.size();
        if ((rv = nng_recv(cmdSock, &in, &in_len, NNG_FLAG_ALLOC)) != 0) {
            LOG_ERROR("cmdSock-nng_recv error: {}", nng_strerror(rv));
            break;
        }
        try {
            // LOG_BUFFER(in, in_len);
            if (dispatcher(in, in_len, cmdBuffer.data(), &out_len)) {
                LOG_DEBUG("Send data length {}", out_len);
                // LOG_BUFFER(cmdBuffer.data(), out_len);
                rv = nng_send(cmdSock, cmdBuffer.data(), out_len, 0);
                if (rv != 0) {
                    LOG_ERROR("cmdSock-nng_send: {}", nng_strerror(rv));
                }

            } else { // Error
                LOG_ERROR("Dispatcher failed...");
                rv = nng_send(cmdSock, cmdBuffer.data(), 0, 0);
                if (rv != 0) {
                    LOG_ERROR("cmdSock-nng_send: {}", nng_strerror(rv));
                }
            }
        } catch (const std::exception &e) {
            LOG_ERROR(GB2312ToUtf8(e.what()));
        } catch (...) {
            LOG_ERROR("Unknow exception.");
        }
        nng_free(in, in_len);
    }
    RpcStopServer();
    LOG_DEBUG("Leave RunRpcServer");
    return rv;
}

int RpcStartServer(int port)
{
    if (isRpcRunning) {
        LOG_WARN("RPC 服务已经启动");
        return 1;
    }

    cmdPort   = port;
    cmdThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunRpcServer, NULL, NULL, NULL);
    if (cmdThread == NULL) {
        LOG_ERROR("CreateThread failed: {}", GetLastError());
        return -1;
    }
#if ENABLE_WX_LOG
    EnableLog();
#endif
    return 0;
}

int RpcStopServer()
{
    if (!isRpcRunning) {
        LOG_WARN("RPC 服务未启动");
        return 1;
    }

    nng_close(cmdSock);
    nng_close(msgSock);
    msgHandler.UnListenPyq();
    msgHandler.UnListenMsg();
#if ENABLE_WX_LOG
    DisableLog();
#endif
    if (cmdThread != NULL) {
        WaitForSingleObject(cmdThread, INFINITE);
        CloseHandle(cmdThread);
        cmdThread = NULL;
    }

    if (msgThread != NULL) {
        WaitForSingleObject(msgThread, INFINITE);
        CloseHandle(msgThread);
        msgThread = NULL;
    }
    isRpcRunning = false;
    return 0;
}
