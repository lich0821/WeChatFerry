#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <unordered_map>

#include <nng/nng.h>

#include "wcf.pb.h"

#include "message_handler.h"
#include "message_sender.h"

class RpcServer
{
public:
    static RpcServer &getInstance();
    static void destroyInstance();

    int start(int port = RPC_DEFAULT_PORT);
    int stop();

private:
    RpcServer(int port = RPC_DEFAULT_PORT);
    ~RpcServer();
    RpcServer(const RpcServer &)            = delete;
    RpcServer &operator=(const RpcServer &) = delete;

    void run_rpc_server();
    void on_message_callback();
    bool start_message_listener(bool pyq, uint8_t *out, size_t *len);
    bool stop_message_listener(uint8_t *out, size_t *len);
    bool dispatcher(uint8_t *in, size_t in_len, uint8_t *out, size_t *out_len);

    static std::string build_url(int port);

    using FunctionHandler = std::function<bool(const Request &, uint8_t *, size_t *)>;

    // 服务器默认端口号和绑定地址
    static constexpr int RPC_DEFAULT_PORT           = 10086;
    static constexpr const char *RPC_SERVER_ADDRESS = "tcp://0.0.0.0";

    int port_ = RPC_DEFAULT_PORT;
    std::atomic<bool> isRunning_ { false };
    std::thread cmdThread_;
    std::thread msgThread_;

    message::Handler &handler_;
    message::Sender &sender_;

    struct Deleter {
        void operator()(RpcServer *server) const { delete server; }
    };

    static std::unique_ptr<RpcServer, Deleter> instance_;
    static const std::unordered_map<Functions, FunctionHandler> rpcFunctionMap;
};
