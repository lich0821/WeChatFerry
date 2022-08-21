#include <stdio.h>
#include <stdlib.h>

#include "accept_new_friend.h"
#include "exec_sql.h"
#include "get_contacts.h"
#include "receive_msg.h"
#include "rpc_h.h"
#include "rpc_server.h"
#include "sdk.h"
#include "send_msg.h"
#include "spy.h"
#include "spy_types.h"
#include "util.h"

using namespace std;

extern int IsLogin(void);                // Defined in spy.cpp
extern HANDLE g_hEvent;                  // New message signal
extern BOOL g_rpcKeepAlive;              // Keep RPC server thread running
extern MsgQueue_t g_MsgQueue;            // Queue for message
extern const MsgTypesMap_t g_WxMsgTypes; // Map of WeChat Message types

static BOOL listenMsgFlag = false;

RPC_STATUS CALLBACK SecurityCallback(RPC_IF_HANDLE /*hInterface*/, void * /*pBindingHandle*/)
{
    return RPC_S_OK; // Always allow anyone.
}

int RpcStartServer()
{
    RPC_STATUS status;
    // Uses the protocol combined with the endpoint for receiving
    // remote procedure calls.
    status = RpcServerUseProtseqEp(reinterpret_cast<RPC_WSTR>((RPC_WSTR)L"ncalrpc"), // Use TCP/IP protocol
                                   RPC_C_LISTEN_MAX_CALLS_DEFAULT,                   // Backlog queue length for TCP/IP.
                                   reinterpret_cast<RPC_WSTR>((RPC_WSTR)L"wcferry"), // TCP/IP port to use
                                   NULL                                              // No security
    );

    if (status)
        return status;

    // Registers the interface and auto listen
    // Equal to RpcServerRegisterIf + RpcServerListen
    status = RpcServerRegisterIf2(server_ISpy_v1_0_s_ifspec, // Interface to register.
                                  NULL,                      // Use the MIDL generated entry-point vector.
                                  NULL,                      // Use the MIDL generated entry-point vector.
                                  RPC_IF_ALLOW_LOCAL_ONLY | RPC_IF_AUTOLISTEN, // Forces use of security callback.
                                  RPC_C_LISTEN_MAX_CALLS_DEFAULT, // Use default number of concurrent calls.
                                  (unsigned)-1,                   // Infinite max size of incoming data blocks.
                                  SecurityCallback);              // Naive security callback.

    while (g_rpcKeepAlive) {
        Sleep(1000); // 休眠，释放CPU
    }

    return 0;
}

int RpcStopServer()
{
    RPC_STATUS status;

    UnListenMessage();

    listenMsgFlag  = false;
    g_rpcKeepAlive = false;
    status         = RpcMgmtStopServerListening(NULL);
    if (status)
        return status;

    status = RpcServerUnregisterIf(server_ISpy_v1_0_s_ifspec, NULL, 0);
    return status;
}

int server_IsLogin() { return IsLogin(); }

void server_EnableReceiveMsg()
{
    unsigned long ulCode = 0;
    ListenMessage();
    listenMsgFlag = true;
    RpcTryExcept
    {
        // 调用客户端的回调函数
        while (listenMsgFlag) {
            // 中断式，兼顾及时性和CPU使用率
            WaitForSingleObject(g_hEvent, INFINITE); // 等待消息
            while (!g_MsgQueue.empty()) {
                client_ReceiveMsg(g_MsgQueue.front()); // 调用接收消息回调
                g_MsgQueue.pop();
            }
            ResetEvent(g_hEvent);
        }
    }
    RpcExcept(1)
    {
        ulCode = RpcExceptionCode();
        printf("server_EnableReceiveMsg exception 0x%lx = %ld\n", ulCode, ulCode);
    }
    RpcEndExcept
}

void server_DisableReceiveMsg()
{
    UnListenMessage();
    listenMsgFlag = false;
}

int server_SendTextMsg(const wchar_t *wxid, const wchar_t *msg, const wchar_t *atWxids)
{
    SendTextMessage(wxid, msg, atWxids);

    return 0;
}

int server_SendImageMsg(const wchar_t *wxid, const wchar_t *path)
{
    SendImageMessage(wxid, path);

    return 0;
}

int server_GetMsgTypes(int *pNum, PPRpcIntBstrPair *msgTypes)
{
    *pNum               = g_WxMsgTypes.size();
    PPRpcIntBstrPair pp = (PPRpcIntBstrPair)midl_user_allocate(*pNum * sizeof(PRpcIntBstrPair));
    if (pp == NULL) {
        printf("server_GetMsgTypes midl_user_allocate Failed for pp\n");
        return -2;
    }
    int index = 0;
    for (auto it = g_WxMsgTypes.begin(); it != g_WxMsgTypes.end(); it++) {
        PRpcIntBstrPair p = (PRpcIntBstrPair)midl_user_allocate(sizeof(RpcIntBstrPair_t));
        if (p == NULL) {
            printf("server_GetMsgTypes midl_user_allocate Failed for p\n");
            return -3;
        }

        p->key      = it->first;
        p->value    = SysAllocString(it->second.c_str());
        pp[index++] = p;
    }

    *msgTypes = pp;

    return 0;
}

int server_GetContacts(int *pNum, PPRpcContact *contacts)
{
    vector<RpcContact_t> vContacts = GetContacts();

    *pNum           = vContacts.size();
    PPRpcContact pp = (PPRpcContact)midl_user_allocate(*pNum * sizeof(PRpcContact));
    if (pp == NULL) {
        printf("server_GetMsgTypes midl_user_allocate Failed for pp\n");
        return -2;
    }

    int index = 0;
    for (auto it = vContacts.begin(); it != vContacts.end(); it++) {
        PRpcContact p = (PRpcContact)midl_user_allocate(sizeof(RpcContact_t));
        if (p == NULL) {
            printf("server_GetMsgTypes midl_user_allocate Failed for p\n");
            return -3;
        }

        p->wxId       = it->wxId;
        p->wxCode     = it->wxCode;
        p->wxName     = it->wxName;
        p->wxCountry  = it->wxCountry;
        p->wxProvince = it->wxProvince;
        p->wxCity     = it->wxCity;
        p->wxGender   = it->wxGender;

        pp[index++] = p;
    }

    *contacts = pp;

    return 0;
}

int server_GetDbNames(int *pNum, BSTR **dbs)
{
    vector<wstring> vDbs = GetDbNames();
    *pNum                = vDbs.size();
    BSTR *pp             = (BSTR *)midl_user_allocate(*pNum * sizeof(BSTR));
    if (pp == NULL) {
        printf("server_GetMsgTypes midl_user_allocate Failed for pp\n");
        return -2;
    }

    int index = 0;
    for (auto it = vDbs.begin(); it != vDbs.end(); it++) {
        pp[index++] = GetBstrFromWstring(*it);
    }

    *dbs = pp;

    return 0;
}

int server_GetDbTables(const wchar_t *db, int *pNum, PPRpcTables *tbls)
{
    vector<RpcTables_t> tables = GetDbTables(db);
    *pNum                      = tables.size();
    PPRpcTables pp             = (PPRpcTables)midl_user_allocate(*pNum * sizeof(PRpcTables));
    if (pp == NULL) {
        printf("server_GetMsgTypes midl_user_allocate Failed for pp\n");
        return -2;
    }

    int index = 0;
    for (auto it = tables.begin(); it != tables.end(); it++) {
        PRpcTables p = (PRpcTables)midl_user_allocate(sizeof(RpcTables_t));
        if (p == NULL) {
            printf("server_GetDbTables midl_user_allocate Failed for p\n");
            return -3;
        }

        p->table    = it->table;
        p->sql      = it->sql;
        pp[index++] = p;
    }

    *tbls = pp;

    return 0;
}

int server_ExecDbQuery(const wchar_t *db, const wchar_t *sql, int *pRow, int *pCol, PPPRpcSqlResult *ret)
{
    vector<vector<RpcSqlResult_t>> vvSqlResult = ExecDbQuery(db, sql);
    if (vvSqlResult.empty()) {
        *pRow = *pCol = 0;
        ret           = NULL;
        return -1;
    }
    *pRow               = vvSqlResult.size();
    *pCol               = vvSqlResult[0].size();
    PPPRpcSqlResult ppp = (PPPRpcSqlResult)midl_user_allocate(*pRow * sizeof(PPRpcSqlResult));
    if (ppp == NULL) {
        printf("server_ExecDbQuery midl_user_allocate Failed for ppp\n");
        return -2;
    }

    for (int r = 0; r < *pRow; r++) {
        PPRpcSqlResult pp = (PPRpcSqlResult)midl_user_allocate(*pCol * sizeof(PRpcSqlResult));
        if (pp == NULL) {
            midl_user_free(ppp);
            printf("server_ExecDbQuery midl_user_allocate Failed for pp\n");
            return -2;
        }

        for (int c = 0; c < *pCol; c++) {
            PRpcSqlResult p = (PRpcSqlResult)midl_user_allocate(sizeof(RpcSqlResult_t));
            if (p == NULL) {
                midl_user_free(pp);
                printf("server_ExecDbQuery midl_user_allocate Failed for p\n");
                return -2;
            }

            p->type    = vvSqlResult[r][c].type;
            p->column  = vvSqlResult[r][c].column;
            p->content = vvSqlResult[r][c].content;

            pp[c] = p;
        }

        ppp[r] = pp;
    }

    *ret = ppp;

    return 0;
}

BOOL server_AcceptNewFriend(const wchar_t *v3, const wchar_t *v4) { return AcceptNewFriend(v3, v4); }
