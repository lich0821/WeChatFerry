/*
RPC Client
*/
#pragma warning(disable : 4251)
#pragma execution_character_set("utf-8")

#include <memory>
#include <signal.h>
#include <thread>
#if 0
#include <grpcpp/grpcpp.h>

#include "../proto/wcf.grpc.pb.h"
#include "../sdk/sdk.h"

using namespace std;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using wcf::Contact;
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

class WcfClient
{
public:
    static WcfClient &Instance(string host_port)
    {
        static WcfClient instance(grpc::CreateChannel(host_port, grpc::InsecureChannelCredentials()));
        return instance;
    }

    ~WcfClient()
    {
        cout << "~WcfClient()" << endl;
        this->DisableRecvMsg();
        WxDestroySDK();
    }

    int IsLogin()
    {
        Empty empty;
        Response rsp;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        stub_->async()->RpcIsLogin(&context, &empty, &rsp, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "IsLogin rpc failed." << endl;
        }

        return rsp.status();
    }

    string GetSelfWxid()
    {
        Empty empty;
        String rsp;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        stub_->async()->RpcGetSelfWxid(&context, &empty, &rsp, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "GetSelfWxid rpc failed." << endl;
        }

        return rsp.str();
    }

    void EnableRecvMsg(function<void(WxMsg &)> msg_handle_cb)
    {
        class Reader : public grpc::ClientReadReactor<WxMsg>
        {
        public:
            Reader(Wcf::Stub *stub, function<void(WxMsg &)> msg_handle_cb)
                : msg_handle_cb_(msg_handle_cb)
            {
                stub->async()->RpcEnableRecvMsg(&context_, &empty_, this);
                StartRead(&msg_);
                StartCall();
            }

            void OnReadDone(bool ok) override
            {
                if (ok) {
                    try {
                        msg_handle_cb_(msg_);
                    } catch (...) {
                        cout << "OnMsg wrong..." << endl;
                    }
                    StartRead(&msg_);
                }
            }

            void OnDone(const Status &s) override
            {
                unique_lock<mutex> l(mu_);
                status_ = s;
                done_   = true;
                cv_.notify_one();
            }

            Status Await()
            {
                unique_lock<mutex> l(mu_);
                cv_.wait(l, [this] { return done_; });
                return move(status_);
            }

        private:
            Empty empty_;
            WxMsg msg_;
            ClientContext context_;

            mutex mu_;
            Status status_;
            bool done_ = false;
            condition_variable cv_;

            function<void(WxMsg &)> msg_handle_cb_;
        };

        Reader reader(stub_.get(), msg_handle_cb);
        Status status = reader.Await();

        if (!status.ok()) {
            cout << "GetMessage rpc failed." << endl;
        }
    }

    int DisableRecvMsg()
    {
        Empty empty;
        Response rsp;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        stub_->async()->RpcDisableRecvMsg(&context, &empty, &rsp, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "DisableRecvMsg rpc failed." << endl;
        }

        return rsp.status();
    }

    int SendTextMsg(string msg, string receiver, string atusers)
    {
        Response rsp;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        TextMsg t_msg;
        t_msg.set_msg(msg);
        t_msg.set_receiver(receiver);
        t_msg.set_aters(atusers);

        stub_->async()->RpcSendTextMsg(&context, &t_msg, &rsp, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "SendTextMsg rpc failed." << endl;
            rsp.set_status(-999); // TODO: Unify error code
        }

        return rsp.status();
    }

    int SendImageMsg(string path, string receiver)
    {
        Response rsp;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        ImageMsg i_msg;
        i_msg.set_path(path);
        i_msg.set_receiver(receiver);

        stub_->async()->RpcSendImageMsg(&context, &i_msg, &rsp, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "SendImageMsg rpc failed." << endl;
            rsp.set_status(-999); // TODO: Unify error code
        }

        return rsp.status();
    }

    MsgTypes GetMsgTypes(void)
    {
        Empty empty;
        MsgTypes mt;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        stub_->async()->RpcGetMsgTypes(&context, &empty, &mt, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "GetMsgTypes rpc failed." << endl;
        }

        return mt;
    }

    Contacts GetContacts(void)
    {
        Empty empty;
        Contacts contacts;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        stub_->async()->RpcGetContacts(&context, &empty, &contacts, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "GetContacts rpc failed." << endl;
        }

        return contacts;
    }

    DbNames GetDbNames(void)
    {
        Empty empty;
        DbNames names;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        stub_->async()->RpcGetDbNames(&context, &empty, &names, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "GetDbNames rpc failed." << endl;
        }

        return names;
    }

    DbTables GetDbTables(string db)
    {
        DbTables tables;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        String s_db;
        s_db.set_str(db);

        stub_->async()->RpcGetDbTables(&context, &s_db, &tables, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "GetDbTables rpc failed." << endl;
        }

        return tables;
    }

    DbRows ExecDbQuery(string db, string sql)
    {
        DbRows rows;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        DbQuery query;
        query.set_db(db);
        query.set_sql(sql);

        stub_->async()->RpcExecDbQuery(&context, &query, &rows, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "ExecDbQuery rpc failed." << endl;
        }

        return rows;
    }

    int AcceptNewFriend(string v3, string v4)
    {
        Response rsp;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        Verification v;
        v.set_v3(v3);
        v.set_v4(v4);

        stub_->async()->RpcAcceptNewFriend(&context, &v, &rsp, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "ExecDbQuery rpc failed." << endl;
            rsp.set_status(-999); // TODO: Unify error code
        }

        return rsp.status();
    }

private:
    unique_ptr<Wcf::Stub> stub_;
    WcfClient(shared_ptr<Channel> channel)
        : stub_(Wcf::NewStub(channel))
    {
        WxInitSDK();
    }
};

int OnMsg(WxMsg msg)
{
    cout << "Got Message: \n"
         << msg.is_self() << ", " << msg.is_group() << ", " << msg.type() << ", " << msg.id() << ", " << msg.xml()
         << ", " << msg.sender() << ", " << msg.roomid() << ", " << msg.content() << endl;
    return 0;
}

volatile sig_atomic_t gStop;
void handler(int s)
{
    cout << "Ctrl + C" << endl;
    gStop = 1;
}

int main(int argc, char **argv)
{
    int ret;

    signal(SIGINT, handler);

    WcfClient &client = WcfClient::Instance("localhost:10086");

    cout << "IsLogin: " << client.IsLogin() << endl;
    cout << "Self Wxid: " << client.GetSelfWxid() << endl;

    ret = client.SendTextMsg("来自CPP的消息！", "filehelper", "");
    cout << "SendTextMsg: " << ret << endl;

    ret = client.SendImageMsg("TEQuant.jpeg", "filehelper");
    cout << "SendImageMsg: " << ret << endl;

    MsgTypes mts = client.GetMsgTypes();
    cout << "GetMsgTypes: " << mts.types_size() << endl;
    map<int32_t, string> m(mts.types().begin(), mts.types().end());
    for (auto &[k, v] : m) {
        cout << k << ": " << v << endl;
    }

    Contacts cnts = client.GetContacts();
    cout << "GetContacts: " << cnts.contacts().size() << endl;
    vector<Contact> vcnts(cnts.contacts().begin(), cnts.contacts().end());
    for (auto &c : vcnts) {
        string gender = "";
        if (c.gender() == 1) {
            gender = "男";
        } else if (c.gender() == 2) {
            gender = "女";
        }
        cout << c.wxid() << "\t" << c.code() << "\t" << c.name() << "\t" << c.country() << "\t" << c.province() << "\t"
             << c.city() << "\t" << gender << endl;
    }

    DbNames db = client.GetDbNames();
    cout << "GetDbNames: " << db.names().size() << endl;
    vector<string> dbs(db.names().begin(), db.names().end());
    for (auto &name : dbs) {
        cout << name << endl;
    }

    DbTables tbls = client.GetDbTables("db");
    cout << "GetDbTables: " << tbls.tables().size() << endl;
    vector<DbTable> vtbls(tbls.tables().begin(), tbls.tables().end());
    for (auto &tbl : vtbls) {
        cout << tbl.name() << "\n" << tbl.sql() << endl;
    }

    DbRows r = client.ExecDbQuery("MicroMsg.db", "SELECT * FROM Contact LIMIT 1;");
    cout << "ExecDbQuery: " << r.rows().size() << endl;
    vector<DbRow> vrows(r.rows().begin(), r.rows().end());
    for (auto &row : vrows) {
        vector<DbField> vfields(row.fields().begin(), row.fields().end());
        for (auto &field : vfields)
            cout << field.column() << "[" << field.type() << "]: " << field.content() << endl;
    }

    // 需要正确的 v3、v4 才能调用
    // ret = client.AcceptNewFriend("v3", "v4");
    // cout << "AcceptNewFriend: " << ret << endl;

    function<void(WxMsg &)> cb = OnMsg;
    thread t1                  = thread([&]() { client.EnableRecvMsg(cb); });

    while (!gStop) {
        Sleep(1000);
    }

    cout << "Cleanup" << endl;
    client.DisableRecvMsg();

    system("pause");
    client.~WcfClient();

    return 0;
}
#endif
int main() { return 0; }
