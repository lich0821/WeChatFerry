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
#include <unordered_map>

#include <magic_enum.hpp>
#include <nng/nng.h>
#include <nng/protocol/pair1/pair.h>
#include <nng/supplemental/util/platform.h>

#include "wcf.pb.h"

#include "account_manager.h"
#include "chatroom_manager.h"
#include "contact_manager.h"
#include "database_executor.h"
#include "log.hpp"
#include "message_receiver.h"
#include "message_sender.h"
#include "misc_manager.h"
#include "pb_types.h"
#include "pb_util.h"
#include "rpc_helper.h"
#include "rpc_server.h"
#include "spy.h"
#include "spy_types.h"
#include "util.h"

#define URL_SIZE   20
#define BASE_URL   "tcp://0.0.0.0"
#define G_BUF_SIZE (16 * 1024 * 1024)

namespace fs = std::filesystem;

extern int IsLogin(void); // Defined in spy.cpp

bool gIsListening    = false;
bool gIsListeningPyq = false;
std::mutex gMutex;
std::condition_variable gCV;
std::queue<WxMsg_t> gMsgQueue;

static int lport       = 0;
static HANDLE lRpcThread = NULL;
static bool lIsRunning = false;
static nng_socket cmdSock, msgSock; // TODO: 断开检测
static uint8_t gBuffer[G_BUF_SIZE] = { 0 };

// RPC 函数映射表 - 使用 lambda 包装调用
using RpcFunctionHandler = std::function<bool(const Request &, uint8_t *, size_t *)>;

static const std::unordered_map<Functions, RpcFunctionHandler> rpcFunctionMap = {
    // clang-format off
    // Account 模块
    { Functions_FUNC_IS_LOGIN, [](const Request &r, uint8_t *out, size_t *len) {
        return account::rpc_is_logged_in(out, len);
    }},
    { Functions_FUNC_GET_SELF_WXID, [](const Request &r, uint8_t *out, size_t *len) {
        return account::rpc_get_self_wxid(out, len);
    }},
    { Functions_FUNC_GET_USER_INFO, [](const Request &r, uint8_t *out, size_t *len) {
        return account::rpc_get_user_info(out, len);
    }},

    // Contact 模块
    { Functions_FUNC_GET_CONTACTS, [](const Request &r, uint8_t *out, size_t *len) {
        return contact::rpc_get_contacts(out, len);
    }},
    { Functions_FUNC_GET_CONTACT_INFO, [](const Request &r, uint8_t *out, size_t *len) {
        return contact::rpc_get_contact_info(r.msg.str, out, len);
    }},
    { Functions_FUNC_ACCEPT_FRIEND, [](const Request &r, uint8_t *out, size_t *len) {
        return contact::rpc_accept_friend(r.msg.v.v3, r.msg.v.v4, r.msg.v.scene, out, len);
    }},

    // Message Handler 模块
    { Functions_FUNC_GET_MSG_TYPES, [](const Request &r, uint8_t *out, size_t *len) {
        return message::rpc_get_msg_types(out, len);
    }},
    { Functions_FUNC_ENABLE_RECV_TXT, [](const Request &r, uint8_t *out, size_t *len) {
        return message::rpc_enable_recv_txt(r.msg.flag, out, len);
    }},
    { Functions_FUNC_DISABLE_RECV_TXT, [](const Request &r, uint8_t *out, size_t *len) {
        return message::rpc_disable_recv_txt(out, len);
    }},

    // Message Sender 模块
    { Functions_FUNC_SEND_TXT, [](const Request &r, uint8_t *out, size_t *len) {
        return message::rpc_send_text(r.msg.txt, out, len);
    }},
    { Functions_FUNC_SEND_IMG, [](const Request &r, uint8_t *out, size_t *len) {
        return message::rpc_send_image(r.msg.file, out, len);
    }},
    { Functions_FUNC_SEND_FILE, [](const Request &r, uint8_t *out, size_t *len) {
        return message::rpc_send_file(r.msg.file, out, len);
    }},
    { Functions_FUNC_SEND_XML, [](const Request &r, uint8_t *out, size_t *len) {
        return message::rpc_send_xml(r.msg.xml, out, len);
    }},
    { Functions_FUNC_SEND_EMOTION, [](const Request &r, uint8_t *out, size_t *len) {
        return message::rpc_send_emotion(r.msg.file, out, len);
    }},
    { Functions_FUNC_SEND_RICH_TXT, [](const Request &r, uint8_t *out, size_t *len) {
        return message::rpc_send_rich_text(r.msg.rt, out, len);
    }},
    { Functions_FUNC_SEND_PAT_MSG, [](const Request &r, uint8_t *out, size_t *len) {
        return message::rpc_send_pat(r.msg.pm, out, len);
    }},
    { Functions_FUNC_FORWARD_MSG, [](const Request &r, uint8_t *out, size_t *len) {
        return message::rpc_forward(r.msg.fm, out, len);
    }},

    // Database Executor 模块
    { Functions_FUNC_GET_DB_NAMES, [](const Request &r, uint8_t *out, size_t *len) {
        return db::rpc_get_db_names(out, len);
    }},
    { Functions_FUNC_GET_DB_TABLES, [](const Request &r, uint8_t *out, size_t *len) {
        return db::rpc_get_db_tables(r.msg.str, out, len);
    }},
    { Functions_FUNC_EXEC_DB_QUERY, [](const Request &r, uint8_t *out, size_t *len) {
        return db::rpc_exec_db_query(r.msg.query, out, len);
    }},

    // Chatroom Manager 模块
    { Functions_FUNC_ADD_ROOM_MEMBERS, [](const Request &r, uint8_t *out, size_t *len) {
        return chatroom::rpc_add_chatroom_member(r.msg.m, out, len);
    }},
    { Functions_FUNC_DEL_ROOM_MEMBERS, [](const Request &r, uint8_t *out, size_t *len) {
        return chatroom::rpc_delete_chatroom_member(r.msg.m, out, len);
    }},
    { Functions_FUNC_INV_ROOM_MEMBERS, [](const Request &r, uint8_t *out, size_t *len) {
        return chatroom::rpc_invite_chatroom_member(r.msg.m, out, len);
    }},

    // Misc Manager 模块
    { Functions_FUNC_GET_AUDIO_MSG, [](const Request &r, uint8_t *out, size_t *len) {
        return misc::rpc_get_audio(r.msg.am, out, len);
    }},
    { Functions_FUNC_RECV_TRANSFER, [](const Request &r, uint8_t *out, size_t *len) {
        return misc::rpc_receive_transfer(r.msg.tf, out, len);
    }},
    { Functions_FUNC_REFRESH_PYQ, [](const Request &r, uint8_t *out, size_t *len) {
        return misc::rpc_refresh_pyq(r.msg.ui64, out, len);
    }},
    { Functions_FUNC_DOWNLOAD_ATTACH, [](const Request &r, uint8_t *out, size_t *len) {
        return misc::rpc_download_attachment(r.msg.att, out, len);
    }},
    { Functions_FUNC_REVOKE_MSG, [](const Request &r, uint8_t *out, size_t *len) {
        return misc::rpc_revoke_message(r.msg.ui64, out, len);
    }},
    { Functions_FUNC_REFRESH_QRCODE, [](const Request &r, uint8_t *out, size_t *len) {
        return misc::rpc_get_login_url(out, len);
    }},
    { Functions_FUNC_DECRYPT_IMAGE, [](const Request &r, uint8_t *out, size_t *len) {
        return misc::rpc_decrypt_image(r.msg.dec, out, len);
    }},
    { Functions_FUNC_EXEC_OCR, [](const Request &r, uint8_t *out, size_t *len) {
        return misc::rpc_get_ocr_result(r.msg.str, out, len);
    }}
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

    // 使用函数映射表分发请求
    auto it = rpcFunctionMap.find(req.func);
    if (it != rpcFunctionMap.end()) {
        ret = it->second(req, out, out_len);
    } else {
        LOG_ERROR("[UNKNOWN FUNCTION]");
    }

    pb_release(Request_fields, &req);
    return ret;
}

static int RunServer()
{
    int rv                 = 0;
    char url[URL_SIZE + 1] = { 0 };
    sprintf_s(url, URL_SIZE, "%s:%d", BASE_URL, lport);
    if ((rv = nng_pair1_open(&cmdSock)) != 0) {
        LOG_ERROR("nng_pair0_open error {}", nng_strerror(rv));
        return rv;
    }

    if ((rv = nng_listen(cmdSock, (char *)url, NULL, 0)) != 0) {
        LOG_ERROR("nng_listen error {}", nng_strerror(rv));
        return rv;
    }

    LOG_INFO("CMD Server listening on {}", (char *)url);
    if ((rv = nng_setopt_ms(cmdSock, NNG_OPT_SENDTIMEO, 1000)) != 0) {
        LOG_ERROR("nng_setopt_ms error: {}", nng_strerror(rv));
        return rv;
    }

    lIsRunning = true;
    while (lIsRunning) {
        uint8_t *in = NULL;
        size_t in_len, out_len = G_BUF_SIZE;
        if ((rv = nng_recv(cmdSock, &in, &in_len, NNG_FLAG_ALLOC)) != 0) {
            LOG_ERROR("cmdSock-nng_recv error: {}", nng_strerror(rv));
            break;
        }
        try {
            // LOG_BUFFER(in, in_len);
            if (dispatcher(in, in_len, gBuffer, &out_len)) {
                LOG_DEBUG("Send data length {}", out_len);
                // LOG_BUFFER(gBuffer, out_len);
                rv = nng_send(cmdSock, gBuffer, out_len, 0);
                if (rv != 0) {
                    LOG_ERROR("cmdSock-nng_send: {}", nng_strerror(rv));
                }

            } else { // Error
                LOG_ERROR("Dispatcher failed...");
                rv = nng_send(cmdSock, gBuffer, 0, 0);
                if (rv != 0) {
                    LOG_ERROR("cmdSock-nng_send: {}", nng_strerror(rv));
                }
                // break;
            }
        } catch (const std::exception &e) {
            LOG_ERROR(util::gb2312_to_utf8(e.what()));
        } catch (...) {
            LOG_ERROR("Unknow exception.");
        }
        nng_free(in, in_len);
    }
    LOG_DEBUG("Leave RunServer");
    return rv;
}

int RpcStartServer(int port)
{
    if (lIsRunning) {
        return 0;
    }

    lport = port;

    lRpcThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunServer, NULL, NULL, NULL);
    if (lRpcThread == NULL) {
        LOG_ERROR("Failed to create RPC thread");
        return -1;
    }

    return 0;
}

int RpcStopServer()
{
    if (!lIsRunning) {
        return 0;
    }

    lIsRunning = false;
    nng_close(cmdSock);
    nng_close(msgSock);
    message::stop_receiving();

    // 等待线程退出
    if (lRpcThread != NULL) {
        WaitForSingleObject(lRpcThread, 5000); // 最多等待 5 秒
        CloseHandle(lRpcThread);
        lRpcThread = NULL;
    }

    LOG_INFO("Server stopped.");
    return 0;
}
