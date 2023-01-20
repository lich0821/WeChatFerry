#pragma warning(disable : 4251)

#include <memory>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <thread>

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "../proto/wcf.grpc.pb.h"

#include "accept_new_friend.h"
#include "exec_sql.h"
#include "get_contacts.h"
#include "log.h"
#include "receive_msg.h"
#include "rpc_server.h"
#include "send_msg.h"
#include "spy.h"
#include "spy_types.h"
#include "util.h"

extern int IsLogin(void);         // Defined in spy.cpp
extern std::string GetSelfWxid(); // Defined in spy.cpp

using namespace std;

using grpc::CallbackServerContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerUnaryReactor;
using grpc::ServerWriteReactor;
using grpc::Status;

using wcf::Contacts;
using wcf::DbField;
using wcf::DbNames;
using wcf::DbQuery;
using wcf::DbRow;
using wcf::DbRows;
using wcf::DbTable;
using wcf::DbTables;
using wcf::Empty;
using wcf::ImageMsg;
using wcf::MsgTypes;
using wcf::Response;
using wcf::String;
using wcf::TextMsg;
using wcf::Verification;
using wcf::Wcf;
using wcf::WxMsg;

mutex gMutex;
queue<WxMsg> gMsgQueue;
condition_variable gCv;
bool gIsListening;

class WcfImpl final : public Wcf::CallbackService
{
public:
    explicit WcfImpl() { }

    ServerUnaryReactor *RpcIsLogin(CallbackServerContext *context, const Empty *empty, Response *rsp) override
    {
        int ret = IsLogin();
        rsp->set_status(ret);
        auto *reactor = context->DefaultReactor();
        reactor->Finish(Status::OK);
        return reactor;
    }

    ServerUnaryReactor *RpcGetSelfWxid(CallbackServerContext *context, const Empty *empty, String *rsp) override
    {
        string wxid = GetSelfWxid();
        rsp->set_str(wxid);
        auto *reactor = context->DefaultReactor();
        reactor->Finish(Status::OK);
        return reactor;
    }

    ServerWriteReactor<WxMsg> *RpcEnableRecvMsg(CallbackServerContext *context, const Empty *empty) override
    {
        class Getter : public ServerWriteReactor<WxMsg>
        {
        public:
            Getter()
            {
                LOG_INFO("Enable message listening.")
                ListenMessage(); // gIsListening = true;
                NextWrite();
            }
            void OnDone() override { delete this; }
            void OnWriteDone(bool /*ok*/) override { NextWrite(); }

        private:
            void NextWrite()
            {
                unique_lock<std::mutex> lock(gMutex);
                gCv.wait(lock, [&] { return !gMsgQueue.empty(); });
                tmp_ = gMsgQueue.front();
                gMsgQueue.pop();
                lock.unlock();
                if (gIsListening) {
                    StartWrite(&tmp_);
                } else {
                    LOG_INFO("Disable message listening.")
                    Finish(Status::OK); // 结束本次通信
                }
            }
            WxMsg tmp_; // 如果将它放到 NextWrite 内部，StartWrite 调用时可能已经出了作用域
        };

        return new Getter();
    }

    ServerUnaryReactor *RpcDisableRecvMsg(CallbackServerContext *context, const Empty *empty, Response *rsp) override
    {
        if (gIsListening) {
            UnListenMessage(); // gIsListening = false;
            // 发送消息，触发 NextWrite 的 Finish
            WxMsg wxMsg;
            unique_lock<std::mutex> lock(gMutex);
            gMsgQueue.push(wxMsg);
            lock.unlock();
            gCv.notify_all();
        }

        rsp->set_status(0);
        auto *reactor = context->DefaultReactor();
        reactor->Finish(Status::OK);
        return reactor;
    }

    ServerUnaryReactor *RpcSendTextMsg(CallbackServerContext *context, const TextMsg *msg, Response *rsp) override
    {
        wstring wswxid    = String2Wstring(msg->receiver());
        wstring wsmsg     = String2Wstring(msg->msg());
        wstring wsatusers = String2Wstring(msg->aters());

        SendTextMessage(wswxid.c_str(), wsmsg.c_str(), wsatusers.c_str());
        rsp->set_status(0);
        auto *reactor = context->DefaultReactor();
        reactor->Finish(Status::OK);
        return reactor;
    }

    ServerUnaryReactor *RpcSendImageMsg(CallbackServerContext *context, const ImageMsg *msg, Response *rsp) override
    {
        wstring wswxid = String2Wstring(msg->receiver());
        wstring wspath = String2Wstring(msg->path());

        SendImageMessage(wswxid.c_str(), wspath.c_str());
        rsp->set_status(0);
        auto *reactor = context->DefaultReactor();
        reactor->Finish(Status::OK);
        return reactor;
    }

    ServerUnaryReactor *RpcGetMsgTypes(CallbackServerContext *context, const Empty *empty, MsgTypes *rsp) override
    {
        GetMsgTypes(rsp);
        auto *reactor = context->DefaultReactor();
        reactor->Finish(Status::OK);

        return reactor;
    }

    ServerUnaryReactor *RpcGetContacts(CallbackServerContext *context, const Empty *empty, Contacts *rsp) override
    {
        bool ret      = GetContacts(rsp);
        auto *reactor = context->DefaultReactor();
        if (ret) {
            reactor->Finish(Status::OK);
        } else {
            reactor->Finish(Status::CANCELLED);
        }

        return reactor;
    }

    ServerUnaryReactor *RpcGetDbNames(CallbackServerContext *context, const Empty *empty, DbNames *rsp) override
    {
        GetDbNames(rsp);
        auto *reactor = context->DefaultReactor();
        reactor->Finish(Status::OK);

        return reactor;
    }

    ServerUnaryReactor *RpcGetDbTables(CallbackServerContext *context, const String *db, DbTables *rsp) override
    {
        GetDbTables(db->str(), rsp);
        auto *reactor = context->DefaultReactor();
        reactor->Finish(Status::OK);

        return reactor;
    }

    ServerUnaryReactor *RpcExecDbQuery(CallbackServerContext *context, const DbQuery *query, DbRows *rsp) override
    {
        ExecDbQuery(query->db(), query->sql(), rsp);
        auto *reactor = context->DefaultReactor();
        reactor->Finish(Status::OK);

        return reactor;
    }

    ServerUnaryReactor *RpcAcceptNewFriend(CallbackServerContext *context, const Verification *v,
                                           Response *rsp) override
    {
        bool ret      = AcceptNewFriend(String2Wstring(v->v3()), String2Wstring(v->v4()));
        auto *reactor = context->DefaultReactor();
        if (ret) {
            rsp->set_status(0);
            reactor->Finish(Status::OK);
        } else {
            LOG_ERROR("AcceptNewFriend failed.")
            rsp->set_status(-1); // TODO: Unify error code
            reactor->Finish(Status::CANCELLED);
        }

        return reactor;
    }
};

static DWORD lThreadId = 0;
static bool lIsRunning = false;
static ServerBuilder lBuilder;
static WcfImpl lService;

static unique_ptr<Server> &GetServer()
{
    static unique_ptr<Server> server(lBuilder.BuildAndStart());

    return server;
}

static int RunServer()
{
    string server_address("0.0.0.0:10086");

    lBuilder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    lBuilder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 2000);
    lBuilder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 3000);
    lBuilder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    lBuilder.RegisterService(&lService);
    
    unique_ptr<Server> &server = GetServer();
    LOG_INFO("Server listening on {}", server_address);
    LOG_DEBUG("server: {}", fmt::ptr(server));
    lIsRunning = true;
    server->Wait();

    return 0;
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
        Empty empty;
        Response rsp;
        CallbackServerContext context;

        unique_ptr<Server> &server = GetServer();
        LOG_DEBUG("server: {}", fmt::ptr(server));
        if (gIsListening) {
            //UnListenMessage();  // Do it in RpcDisableRecvMsg
            lService.RpcDisableRecvMsg(&context, &empty, &rsp);
        }
        server->Shutdown();
        LOG_INFO("Server stoped.");
        lIsRunning = false;
    }

    return 0;
}
