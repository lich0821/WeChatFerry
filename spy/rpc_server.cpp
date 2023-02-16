#pragma warning(disable : 4251)

#include <memory>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <thread>

#include <nng/nng.h>
#include <nng/protocol/pair1/pair.h>
#include <nng/supplemental/util/platform.h>

#include "wcf.pb.h"

#include "accept_new_friend.h"
#include "exec_sql.h"
#include "get_contacts.h"
#include "log.h"
#include "pb_util.h"
#include "receive_msg.h"
#include "rpc_server.h"
#include "send_msg.h"
#include "spy.h"
#include "spy_types.h"
#include "util.h"

#define G_BUF_SIZE (1024 * 1024)

extern int IsLogin(void);         // Defined in spy.cpp
extern std::string GetSelfWxid(); // Defined in spy.cpp

using namespace std;

mutex gMutex;
queue<WxMsg> gMsgQueue;
condition_variable gCv;
bool gIsListening;

static DWORD lThreadId = 0;
static bool lIsRunning = false;
static nng_socket sock;
static uint8_t gBuffer[G_BUF_SIZE] = { 0 };

bool func_is_login(uint8_t *out, size_t *len)
{
    Response rsp   = Response_init_default;
    rsp.func       = Functions_FUNC_IS_LOGIN;
    rsp.which_msg  = Response_status_tag;
    rsp.msg.status = IsLogin();

    pb_ostream_t stream = pb_ostream_from_buffer(out, *len);
    if (!pb_encode(&stream, Response_fields, &rsp)) {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }
    *len = stream.bytes_written;

    return true;
}

bool func_get_self_wxid(uint8_t *out, size_t *len)
{
    Response rsp  = Response_init_default;
    rsp.func      = Functions_FUNC_IS_LOGIN;
    rsp.which_msg = Response_str_tag;
    rsp.msg.str   = (char *)GetSelfWxid().c_str();

    pb_ostream_t stream = pb_ostream_from_buffer(out, *len);
    if (!pb_encode(&stream, Response_fields, &rsp)) {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }
    *len = stream.bytes_written;

    return true;
}

bool func_get_msg_types(uint8_t *out, size_t *len)
{
    Response rsp  = Response_init_default;
    rsp.func      = Functions_FUNC_GET_MSG_TYPES;
    rsp.which_msg = Response_types_tag;

    MsgTypes_t types                 = GetMsgTypes();
    rsp.msg.types.types.funcs.encode = encode_types;
    rsp.msg.types.types.arg          = &types;

    pb_ostream_t stream = pb_ostream_from_buffer(out, *len);
    if (!pb_encode(&stream, Response_fields, &rsp)) {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }
    *len = stream.bytes_written;

    return true;
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

    LOG_INFO("Func: {}", (uint8_t)req.func);
    switch (req.func) {
        case Functions_FUNC_IS_LOGIN: {
            LOG_INFO("[Functions_FUNC_IS_LOGIN]");
            ret = func_is_login(out, out_len);
            break;
        }
        case Functions_FUNC_GET_SELF_WXID: {
            LOG_INFO("[Functions_FUNC_GET_SELF_WXID]");
            ret = func_get_self_wxid(out, out_len);
            break;
        }
        case Functions_FUNC_GET_MSG_TYPES: {
            LOG_INFO("[Functions_FUNC_GET_MSG_TYPES]");
            ret = func_get_msg_types(out, out_len);
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

static int RunServer()
{
    int rv    = 0;
    char *url = (char *)"tcp://0.0.0.0:10086";
    if ((rv = nng_pair1_open(&sock)) != 0) {
        LOG_ERROR("nng_pair0_open error {}", rv);
        return rv;
    }

    if ((rv = nng_listen(sock, url, NULL, 0)) != 0) {
        LOG_ERROR("nng_listen error {}", rv);
        return rv;
    }

    LOG_INFO("Server listening on {}", url);
    if ((rv = nng_setopt_ms(sock, NNG_OPT_SENDTIMEO, 1000)) != 0) {
        LOG_ERROR("nng_recv: {}", rv);
        return rv;
    }

    lIsRunning = true;
    while (lIsRunning) {
        uint8_t *in = NULL;
        size_t in_len, out_len = G_BUF_SIZE;
        if ((rv = nng_recv(sock, &in, &in_len, NNG_FLAG_ALLOC)) != 0) {
            LOG_ERROR("nng_recv: {}", rv);
            break;
        }

        log_buffer(in, in_len);
        if (dispatcher(in, in_len, gBuffer, &out_len)) {
            log_buffer(gBuffer, out_len);
            rv = nng_send(sock, gBuffer, out_len, 0);
            if (rv != 0) {
                LOG_ERROR("nng_send: {}", rv);
            }

        } else {
            // Error
            LOG_ERROR("Dispatcher failed...");
            rv = nng_send(sock, gBuffer, 0, 0);
            break;
        }
        nng_free(in, in_len);
    }
    LOG_INFO("Leave RunServer");
    return rv;
}

int RpcStartServer()
{
    if (lIsRunning) {
        return 0;
    }

    HANDLE rpcThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunServer, NULL, NULL, &lThreadId);
    if (rpcThread != 0) {
        CloseHandle(rpcThread);
    }

    return 0;
}

int RpcStopServer()
{
    if (lIsRunning) {
        nng_close(sock);
        // UnListenMessage();  // Do it in RpcDisableRecvMsg
        LOG_INFO("Server stoped.");
        lIsRunning = false;
    }
    return 0;
}
