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
#include <nng/protocol/pair1/pair.h>
#include <nng/supplemental/util/platform.h>

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

std::unique_ptr<RpcServer, RpcServer::Deleter> RpcServer::instance_ = nullptr;

RpcServer &RpcServer::getInstance()
{
    if (!instance_) {
        instance_.reset(new RpcServer());
    }
    return *instance_;
}

void RpcServer::destroyInstance()
{
    if (instance_) {
        instance_->stop();
        instance_.reset();
    }
}

RpcServer::RpcServer(int port)
    : port_(port), handler_(message::Handler::getInstance()), sender_(message::Sender::get_instance())
{
    LOG_DEBUG("RpcServer 构造: 端口 {}", port_);
}

RpcServer::~RpcServer()
{
    stop();
    LOG_DEBUG("RpcServer 被析构，释放所有资源");
}

std::string RpcServer::build_url(int port)
{
    return std::string(RpcServer::RPC_SERVER_ADDRESS) + ":" + std::to_string(port);
}

int RpcServer::start(int port)
{
    if (isRunning_.load()) {
        LOG_WARN("RPC 服务已在运行");
        return 1;
    }

    port_      = port;
    isRunning_ = true;

    try {
        cmdThread_ = std::thread(&RpcServer::run_rpc_server, this);
    } catch (const std::exception &e) {
        LOG_ERROR("启动 RPC 服务器失败: {}", e.what());
        isRunning_ = false;
        return -2;
    }
#if ENABLE_WX_LOG
    handler_.EnableLog();
#endif
    LOG_INFO("RPC 服务器成功启动，监听端口: {}", port_);
    return 0;
}

int RpcServer::stop()
{
    if (!isRunning_.load()) {
        LOG_WARN("RPC 服务未运行");
        return 1;
    }
    isRunning_ = false;

    handler_.UnListenPyq();
    handler_.UnListenMsg();
#if ENABLE_WX_LOG
    handler_.DisableLog();
#endif
    nng_fini();
    if (cmdThread_.joinable()) {
        LOG_DEBUG("等待命令线程关闭");
        cmdThread_.join();
    }
    LOG_DEBUG("命令线程已经关闭");

    if (msgThread_.joinable()) {
        LOG_DEBUG("等待消息线程关闭");
        msgThread_.join();
    }
    LOG_DEBUG("消息线程已经关闭");
    LOG_INFO("RPC 服务已停止");
    return 0;
}

void RpcServer::on_message_callback()
{
    try {
        int rv;
        nng_socket msgSock = NNG_SOCKET_INITIALIZER;
        Response rsp       = Response_init_default;
        rsp.func           = Functions_FUNC_ENABLE_RECV_TXT;
        rsp.which_msg      = Response_wxmsg_tag;
        std::vector<uint8_t> msgBuffer(DEFAULT_BUF_SIZE);

        pb_ostream_t stream = pb_ostream_from_buffer(msgBuffer.data(), msgBuffer.size());

        std::string url = build_url(port_ + 1);
        if ((rv = nng_pair1_open(&msgSock)) != 0) {
            LOG_ERROR("nng_pair0_open error {}", nng_strerror(rv));
            return;
        }

        if ((rv = nng_listen(msgSock, url.c_str(), NULL, 0)) != 0) {
            LOG_ERROR("nng_listen error {}", nng_strerror(rv));
            return;
        }

        if ((rv = nng_setopt_ms(msgSock, NNG_OPT_SENDTIMEO, 5000)) != 0) {
            LOG_ERROR("nng_setopt_ms: {}", nng_strerror(rv));
            return;
        }

        while (handler_.isMessageListening()) {
            std::optional<WxMsg_t> msgOpt;
            {
                std::unique_lock<std::mutex> lock(handler_.getMutex());
                bool hasMessage
                    = handler_.getConditionVariable().wait_for(lock, std::chrono::milliseconds(1000), [&]() {
                          lock.unlock();
                          msgOpt = handler_.popMessage();
                          lock.lock();
                          return msgOpt.has_value();
                      });

                if (!hasMessage) {
                    continue;
                }
            }

            if (!msgOpt.has_value()) {
                LOG_WARN("popMessage returned empty after wait_for success.");
                continue;
            }

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
        nng_close(msgSock);
        LOG_DEBUG("Leave MSG Server.");
    } catch (const std::exception &e) {
        LOG_ERROR("Fatal exception in on_message_callback: {}", e.what());
    } catch (...) {
        LOG_ERROR("Unknown fatal exception in on_message_callback.");
    }
}

bool RpcServer::start_message_listener(bool pyq, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_ENABLE_RECV_TXT>(out, len, [&](Response &rsp) {
        rsp.msg.status = handler_.ListenMsg();
        if (rsp.msg.status >= 0) {
            if (pyq) {
                handler_.ListenPyq();
            }
            msgThread_ = std::thread(&RpcServer::on_message_callback, this);
        }
    });
}

bool RpcServer::stop_message_listener(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_DISABLE_RECV_TXT>(out, len, [&](Response &rsp) {
        rsp.msg.status = handler_.UnListenMsg();
        if (rsp.msg.status >= 0) {
            handler_.UnListenPyq();
            if (msgThread_.joinable()) {
                msgThread_.join();
            }
        }
    });
}

const std::unordered_map<Functions, RpcServer::FunctionHandler> RpcServer::rpcFunctionMap = {
    // clang-format off
    { Functions_FUNC_IS_LOGIN, [](const Request &r, uint8_t *out, size_t *len) { return account::rpc_is_logged_in(out, len); } },
    { Functions_FUNC_GET_SELF_WXID, [](const Request &r, uint8_t *out, size_t *len) { return account::rpc_get_self_wxid(out, len); } },
    { Functions_FUNC_GET_USER_INFO, [](const Request &r, uint8_t *out, size_t *len) { return account::rpc_get_user_info(out, len); } },
    { Functions_FUNC_GET_MSG_TYPES, [](const Request &r, uint8_t *out, size_t *len) { return RpcServer::getInstance().handler_.rpc_get_msg_types(out, len); } },
    { Functions_FUNC_ENABLE_RECV_TXT, [](const Request &r, uint8_t *out, size_t *len) { return RpcServer::getInstance().start_message_listener(r.msg.flag, out, len); } },
    { Functions_FUNC_DISABLE_RECV_TXT, [](const Request &r, uint8_t *out, size_t *len) { return RpcServer::getInstance().stop_message_listener(out, len); } },
    { Functions_FUNC_GET_CONTACTS, [](const Request &r, uint8_t *out, size_t *len) { return contact::rpc_get_contacts(out, len); } },
    { Functions_FUNC_GET_DB_NAMES, [](const Request &r, uint8_t *out, size_t *len) { return db::rpc_get_db_names(out, len); } },
    { Functions_FUNC_GET_DB_TABLES, [](const Request &r, uint8_t *out, size_t *len) { return db::rpc_get_db_tables(r.msg.str, out, len); } },
    { Functions_FUNC_GET_AUDIO_MSG, [](const Request &r, uint8_t *out, size_t *len) { return misc::rpc_get_audio(r.msg.am, out, len); } },
    { Functions_FUNC_SEND_TXT, [](const Request &r, uint8_t *out, size_t *len) { return RpcServer::getInstance().sender_.rpc_send_text(r.msg.txt, out, len); } },
    { Functions_FUNC_SEND_IMG, [](const Request &r, uint8_t *out, size_t *len) { return RpcServer::getInstance().sender_.rpc_send_image(r.msg.file, out, len); } },
    { Functions_FUNC_SEND_FILE, [](const Request &r, uint8_t *out, size_t *len) { return RpcServer::getInstance().sender_.rpc_send_file(r.msg.file, out, len); } },
    { Functions_FUNC_SEND_XML, [](const Request &r, uint8_t *out, size_t *len) { return RpcServer::getInstance().sender_.rpc_send_xml(r.msg.xml, out, len); } },
    { Functions_FUNC_SEND_EMOTION, [](const Request &r, uint8_t *out, size_t *len) { return RpcServer::getInstance().sender_.rpc_send_emotion(r.msg.file, out, len); } },
    { Functions_FUNC_SEND_RICH_TXT, [](const Request &r, uint8_t *out, size_t *len) { return RpcServer::getInstance().sender_.rpc_send_rich_text(r.msg.rt, out, len); } },
    { Functions_FUNC_SEND_PAT_MSG, [](const Request &r, uint8_t *out, size_t *len) { return RpcServer::getInstance().sender_.rpc_send_pat(r.msg.pm, out, len); } },
    { Functions_FUNC_FORWARD_MSG, [](const Request &r, uint8_t *out, size_t *len) { return RpcServer::getInstance().sender_.rpc_forward(r.msg.fm, out, len); } },
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

bool RpcServer::dispatcher(uint8_t *in, size_t in_len, uint8_t *out, size_t *out_len)
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

    auto it = RpcServer::rpcFunctionMap.find(req.func);
    if (it != RpcServer::rpcFunctionMap.end()) {
        ret = it->second(req, out, out_len);
    } else {
        LOG_ERROR("[未知方法]");
    }

    pb_release(Request_fields, &req);
    return ret;
}

void RpcServer::run_rpc_server()
{
    int rv             = 0;
    nng_socket cmdSock = NNG_SOCKET_INITIALIZER;
    std::string url    = build_url(port_);

    if ((rv = nng_pair1_open(&cmdSock)) != 0) {
        LOG_ERROR("nng_pair1_open error: {}", nng_strerror(rv));
        return;
    }

    if ((rv = nng_listen(cmdSock, url.c_str(), nullptr, 0)) != 0) {
        LOG_ERROR("nng_listen error: {}", nng_strerror(rv));
        nng_close(cmdSock);
        nng_fini();
        return;
    }

    if ((rv = nng_setopt_ms(cmdSock, NNG_OPT_SENDTIMEO, 1000)) != 0) {
        LOG_ERROR("nng_setopt_ms error: {}", nng_strerror(rv));
        nng_close(cmdSock);
        nng_fini();
        return;
    }

    LOG_INFO("CMD Server listening on {}", url);
    std::vector<uint8_t> cmdBuffer(DEFAULT_BUF_SIZE);

    while (isRunning_.load()) {
        uint8_t *in = nullptr;
        size_t in_len, out_len = cmdBuffer.size();

        rv = nng_recv(cmdSock, &in, &in_len, NNG_FLAG_ALLOC);
        if (rv != 0) {
            LOG_ERROR("cmdSock-nng_recv error: {}", nng_strerror(rv));
            break;
        }

        try {
            if (dispatcher(in, in_len, cmdBuffer.data(), &out_len)) {
                LOG_DEBUG("Send data length {}", out_len);

                rv = nng_send(cmdSock, cmdBuffer.data(), out_len, 0);
                if (rv != 0) {
                    LOG_ERROR("cmdSock-nng_send: {}", nng_strerror(rv));
                }
            } else { // 处理失败情况
                LOG_ERROR("Dispatcher failed...");
                rv = nng_send(cmdSock, cmdBuffer.data(), 0, 0);
                if (rv != 0) {
                    LOG_ERROR("cmdSock-nng_send: {}", nng_strerror(rv));
                }
            }
        } catch (const std::exception &e) {
            LOG_ERROR(util::gb2312_to_utf8(e.what()));
        } catch (...) {
            LOG_ERROR("Unknown exception.");
        }

        nng_free(in, in_len);
    }
    nng_close(cmdSock);
    LOG_DEBUG("Leave RunRpcServer");
}
