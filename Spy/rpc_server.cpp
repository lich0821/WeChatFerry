#include <stdio.h>
#include <stdlib.h>

#include "get_contacts.h"
#include "monitor.h"
#include "rpc_server.h"
#include "sdk.h"
#include "send_msg.h"
#include "spy_types.h"

#include "rpc_h.h"
#pragma comment(lib, "Rpcrt4.lib")

extern HANDLE g_hEvent;
extern MsgQueue_t g_MsgQueue;
extern const MsgTypesMap_t g_WxMsgTypes;

int server_IsLogin() { return IsLogin(); }

void server_EnableReceiveMsg()
{
    unsigned long ulCode = 0;

    RpcTryExcept
    {
        // 调用客户端的回调函数
        while (true) {
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

int server_SendTextMsg(const wchar_t *wxid, const wchar_t *at_wxid, const wchar_t *msg)
{
    SendTextMessage(wxid, at_wxid, msg);

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
    PPRpcIntBstrPair pp = (PPRpcIntBstrPair)midl_user_allocate(*pNum * sizeof(RpcIntBstrPair_t));
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
    std::vector<RpcContact_t> vContacts = GetContacts();

    *pNum           = vContacts.size();
    PPRpcContact pp = (PPRpcContact)midl_user_allocate(*pNum * sizeof(RpcContact_t));
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

RPC_STATUS CALLBACK SecurityCallback(RPC_IF_HANDLE /*hInterface*/, void * /*pBindingHandle*/)
{
    return RPC_S_OK; // Always allow anyone.
}

int RpcStartServer(HMODULE hModule)
{
    RPC_STATUS status;
    // Uses the protocol combined with the endpoint for receiving
    // remote procedure calls.
    status = RpcServerUseProtseqEp(reinterpret_cast<RPC_WSTR>((RPC_WSTR)L"ncalrpc"), // Use TCP/IP protocol
                                   RPC_C_LISTEN_MAX_CALLS_DEFAULT,                   // Backlog queue length for TCP/IP.
                                   reinterpret_cast<RPC_WSTR>((RPC_WSTR)L"tmp_endpoint"), // TCP/IP port to use
                                   NULL                                                   // No security
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

    while (1) {
        Sleep(10000); // 休眠，释放CPU
    }

    return 0;
}

int RpcStopServer(void)
{
    RPC_STATUS status;
    status = RpcMgmtStopServerListening(NULL);
    if (status)
        return status;

    status = RpcServerUnregisterIf(NULL, NULL, FALSE);
    return status;
}
