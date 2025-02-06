#pragma warning(disable : 4251)

#include "rpc_server.h"

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

#include "account_manager.h"
#include "chatroom_manager.h"
#include "contact_manager.h"
#include "database_executor.h"
#include "log.hpp"
#include "message_handler.h"
#include "message_sender.h"
#include "misc_manager.h"
#include "pb_types.h"
#include "pb_util.h"
#include "rpc_helper.h"
#include "spy.h"
#include "spy_types.h"
#include "util.h"

namespace fs = std::filesystem;

constexpr size_t DEFAULT_BUF_SIZE = 16 * 1024 * 1024;

static int cmdPort        = 0;
static bool isRpcRunning  = false;
static HANDLE cmdThread   = NULL;
static HANDLE msgThread   = NULL;
static nng_socket cmdSock = NNG_SOCKET_INITIALIZER; // TODO: 断开检测
static nng_socket msgSock = NNG_SOCKET_INITIALIZER; // TODO: 断开检测
auto &handler             = message::Handler::getInstance();
auto &sender              = message::Sender::get_instance();

using FunctionHandler = std::function<bool(const Request &, uint8_t *, size_t *)>;

inline std::string build_url(int port) { return "tcp://0.0.0.0:" + std::to_string(port); }

static void receive_message_callback()
{
    int rv;
    Response rsp  = Response_init_default;
    rsp.func      = Functions_FUNC_ENABLE_RECV_TXT;
    rsp.which_msg = Response_wxmsg_tag;
    std::vector<uint8_t> msgBuffer(DEFAULT_BUF_SIZE);

    pb_ostream_t stream = pb_ostream_from_buffer(msgBuffer.data(), msgBuffer.size());

    std::string url = build_url(cmdPort + 1);
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

    while (handler.isMessageListening()) {
        std::unique_lock<std::mutex> lock(handler.getMutex());
        std::optional<WxMsg_t> msgOpt;
        auto hasMessage = [&]() {
            msgOpt = handler.popMessage();
            return msgOpt.has_value();
        };

        if (handler.getConditionVariable().wait_for(lock, std::chrono::milliseconds(1000), hasMessage)) {
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

static bool rpc_enable_recv_msg(bool pyq, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_ENABLE_RECV_TXT>(out, len, [&](Response &rsp) {
        rsp.msg.status = handler.ListenMsg();
        if (rsp.msg.status == 0) {
            if (pyq) {
                handler.ListenPyq();
            }
            msgThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)receive_message_callback, nullptr, 0, nullptr);
            if (msgThread == nullptr) {
                rsp.msg.status = GetLastError();
                LOG_ERROR("func_enable_recv_txt failed: {}", rsp.msg.status);
            }
        }
    });
}

static bool rpc_disable_recv_msg(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_DISABLE_RECV_TXT>(out, len, [](Response &rsp) {
        rsp.msg.status = handler.UnListenMsg();
        if (rsp.msg.status == 0) {
            handler.UnListenPyq();
            if (msgThread != nullptr) {
                TerminateThread(msgThread, 0);
                msgThread = nullptr;
            }
        }
    });
}

const std::unordered_map<Functions, FunctionHandler> rpc_function_map = {
    // clang-format off
    { Functions_FUNC_IS_LOGIN, [](const Request &r, uint8_t *out, size_t *len) { return misc::rpc_is_logged_in(out, len); } },
    { Functions_FUNC_GET_SELF_WXID, [](const Request &r, uint8_t *out, size_t *len) { return account::rpc_get_self_wxid(out, len); } },
    { Functions_FUNC_GET_USER_INFO, [](const Request &r, uint8_t *out, size_t *len) { return account::rpc_get_user_info(out, len); } },
    { Functions_FUNC_GET_MSG_TYPES, [](const Request &r, uint8_t *out, size_t *len) { return handler.rpc_get_msg_types(out, len); } },
    { Functions_FUNC_GET_CONTACTS, [](const Request &r, uint8_t *out, size_t *len) { return contact::rpc_get_contacts(out, len); } },
    { Functions_FUNC_GET_DB_NAMES, [](const Request &r, uint8_t *out, size_t *len) { return db::rpc_get_db_names(out, len); } },
    { Functions_FUNC_GET_DB_TABLES, [](const Request &r, uint8_t *out, size_t *len) { return db::rpc_get_db_tables(r.msg.str, out, len); } },
    { Functions_FUNC_GET_AUDIO_MSG, [](const Request &r, uint8_t *out, size_t *len) { return misc::rpc_get_audio(r.msg.am, out, len); } },
    { Functions_FUNC_SEND_TXT, [](const Request &r, uint8_t *out, size_t *len) { return sender.rpc_send_text(r.msg.txt, out, len); } },
    { Functions_FUNC_SEND_IMG, [](const Request &r, uint8_t *out, size_t *len) { return sender.rpc_send_image(r.msg.file, out, len); } },
    { Functions_FUNC_SEND_FILE, [](const Request &r, uint8_t *out, size_t *len) { return sender.rpc_send_file(r.msg.file, out, len); } },
    { Functions_FUNC_SEND_XML, [](const Request &r, uint8_t *out, size_t *len) { return sender.rpc_send_xml(r.msg.xml, out, len); } },
    { Functions_FUNC_SEND_EMOTION, [](const Request &r, uint8_t *out, size_t *len) { return sender.rpc_send_emotion(r.msg.file, out, len); } },
    { Functions_FUNC_SEND_RICH_TXT, [](const Request &r, uint8_t *out, size_t *len) { return sender.rpc_send_rich_text(r.msg.rt, out, len); } },
    { Functions_FUNC_SEND_PAT_MSG, [](const Request &r, uint8_t *out, size_t *len) { return sender.rpc_send_pat(r.msg.pm, out, len); } },
    { Functions_FUNC_FORWARD_MSG, [](const Request &r, uint8_t *out, size_t *len) { return sender.rpc_forward(r.msg.fm, out, len); } },
    { Functions_FUNC_ENABLE_RECV_TXT, [](const Request &r, uint8_t *out, size_t *len) { return rpc_enable_recv_msg(r.msg.flag, out, len); } },
    { Functions_FUNC_DISABLE_RECV_TXT, [](const Request &r, uint8_t *out, size_t *len) { return rpc_disable_recv_msg(out, len); } },
    { Functions_FUNC_EXEC_DB_QUERY, [](const Request &r, uint8_t *out, size_t *len) { return db::rpc_exec_db_query(r.msg.query, out, len); } },
    { Functions_FUNC_ACCEPT_FRIEND, [](const Request &r, uint8_t *out, size_t *len) { return contact::rpc_accept_friend(r.msg.v, out, len); } },
    { Functions_FUNC_RECV_TRANSFER, [](const Request &r, uint8_t *out, size_t *len) { return misc::rpc_receive_transfer(r.msg.tf, out, len); } },
    { Functions_FUNC_REFRESH_PYQ, [](const Request &r, uint8_t *out, size_t *len) { return misc::rpc_refresh_pyq(r.msg.ui64, out, len); } },
    { Functions_FUNC_DOWNLOAD_ATTACH, [](const Request &r, uint8_t *out, size_t *len) { return misc::rpc_download_attachment(r.msg.att, out, len); } },
    { Functions_FUNC_GET_CONTACT_INFO, [](const Request &r, uint8_t *out, size_t *len) { return contact::rpc_get_contact_info(r.msg.str, out, len); } },
    { Functions_FUNC_REVOKE_MSG, [](const Request &r, uint8_t *out, size_t *len) { return misc::rpc_revoke_message(r.msg.ui64, out, len); } },
    { Functions_FUNC_REFRESH_QRCODE, [](const Request &r, uint8_t *out, size_t *len) { return misc::rpc_get_login_url(out, len); } },
    { Functions_FUNC_DECRYPT_IMAGE, [](const Request &r, uint8_t *out, size_t *len) { return misc::rpc_decrypt_image(r.msg.dec, out, len); } },
    { Functions_FUNC_EXEC_OCR, [](const Request &r, uint8_t *out, size_t *len) { return misc::rpc_get_ocr_result(r.msg.str, out, len); } },
    { Functions_FUNC_ADD_ROOM_MEMBERS, [](const Request &r, uint8_t *out, size_t *len) { return chatroom::rpc_add_chatroom_member(r.msg.m, out, len); } },
    { Functions_FUNC_DEL_ROOM_MEMBERS, [](const Request &r, uint8_t *out, size_t *len) { return chatroom::rpc_delete_chatroom_member(r.msg.m, out, len); } },
    { Functions_FUNC_INV_ROOM_MEMBERS, [](const Request &r, uint8_t *out, size_t *len) { return chatroom::rpc_invite_chatroom_member(r.msg.m, out, len); } },
    // clang-format on
};

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

    auto it = rpc_function_map.find(req.func);
    if (it != rpc_function_map.end()) {
        ret = it->second(req, out, out_len);
    } else {
        LOG_ERROR("[未知方法]");
    }

    pb_release(Request_fields, &req);
    return ret;
}

static int RunRpcServer()
{
    int rv          = 0;
    std::string url = build_url(cmdPort);
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
            LOG_ERROR(util::gb2312_to_utf8(e.what()));
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
    handler.UnListenPyq();
    handler.UnListenMsg();
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
