#include "rpc_client.h"
#include "sdk.h"
#include "util.h"

static RPC_WSTR pszStringBinding = NULL;
extern std::function<int(WxMessage_t)> g_cbReceiveTextMsg;

RPC_STATUS RpcConnectServer()
{
    RPC_STATUS status = 0;
    // Creates a string binding handle.
    status = RpcStringBindingCompose(NULL,                                             // UUID to bind to
                                     reinterpret_cast<RPC_WSTR>((RPC_WSTR)L"ncalrpc"), // Use TCP/IP protocol
                                     NULL,                                             // TCP/IP network address to use
                                     reinterpret_cast<RPC_WSTR>((RPC_WSTR)L"wcferry"), // TCP/IP port to use
                                     NULL,               // Protocol dependent network options to use
                                     &pszStringBinding); // String binding output

    if (status)
        return status;

    /* Validates the format of the string binding handle and converts it to a binding handle.
    pszStringBinding: The string binding to validate
    hSpyBinding: Put the result in the implicit binding(defined in the IDL file)
    */
    status = RpcBindingFromStringBinding(pszStringBinding, &hSpyBinding);

    return status;
}

RPC_STATUS RpcDisconnectServer()
{
    RPC_STATUS status;
    // Free the memory allocated by a string
    status = RpcStringFree(&pszStringBinding);

    // Releases binding handle resources and disconnects from the server
    status = RpcBindingFree(&hSpyBinding);

    return status;
}

int RpcEnableReceiveMsg()
{
    unsigned long ulCode = 0;
    RpcTryExcept
    {
        // 建立RPC通道，让服务端能够调用客户端的回调函数。（该接口会被服务端阻塞直到异常退出）
        client_EnableReceiveMsg();
    }
    RpcExcept(1)
    {
        ulCode = RpcExceptionCode();
        printf("RpcEnableReceiveMsg exception 0x%lx = %ld\n", ulCode, ulCode);
    }
    RpcEndExcept;

    return 0;
}

int RpcDisableReceiveMsg()
{
    unsigned long ulCode = 0;
    RpcTryExcept
    {
        // UnHook Message receiving
        client_DisableReceiveMsg();
    }
    RpcExcept(1)
    {
        ulCode = RpcExceptionCode();
        printf("RpcDisableReceiveMsg exception 0x%lx = %ld\n", ulCode, ulCode);
    }
    RpcEndExcept;

    return 0;
}

int RpcIsLogin()
{
    int loginFlag        = 0;
    unsigned long ulCode = 0;
    RpcTryExcept
    {
        // 查询登录状态
        loginFlag = client_IsLogin();
    }
    RpcExcept(1)
    {
        ulCode = RpcExceptionCode();
        printf("RpcIsLogin exception 0x%lx = %ld\n", ulCode, ulCode);
        return -1;
    }
    RpcEndExcept;

    return loginFlag;
}

int RpcSendTextMsg(const wchar_t *wxid, const wchar_t *msg, const wchar_t *atWxids)
{
    int ret              = 0;
    unsigned long ulCode = 0;

    RpcTryExcept { ret = client_SendTextMsg(wxid, msg, atWxids); }
    RpcExcept(1)
    {
        ulCode = RpcExceptionCode();
        printf("RpcSendTextMsg exception 0x%lx = %ld\n", ulCode, ulCode);
    }
    RpcEndExcept;

    return ret;
}

int RpcSendImageMsg(const wchar_t *wxid, const wchar_t *path)
{
    int ret              = 0;
    unsigned long ulCode = 0;

    RpcTryExcept { ret = client_SendImageMsg(wxid, path); }
    RpcExcept(1)
    {
        ulCode = RpcExceptionCode();
        printf("RpcSendImageMsg exception 0x%lx = %ld\n", ulCode, ulCode);
    }
    RpcEndExcept;

    return ret;
}

PPRpcIntBstrPair RpcGetMsgTypes(int *pNum)
{
    int ret                        = 0;
    unsigned long ulCode           = 0;
    PPRpcIntBstrPair ppRpcMsgTypes = NULL;

    RpcTryExcept { ret = client_GetMsgTypes(pNum, &ppRpcMsgTypes); }
    RpcExcept(1)
    {
        ulCode = RpcExceptionCode();
        printf("RpcGetMsgTypes exception 0x%lx = %ld\n", ulCode, ulCode);
    }
    RpcEndExcept;
    if (ret != 0) {
        printf("GetMsgTypes Failed: %d\n", ret);
        return NULL;
    }

    return ppRpcMsgTypes;
}

PPRpcContact RpcGetContacts(int *pNum)
{
    int ret                    = 0;
    unsigned long ulCode       = 0;
    PPRpcContact ppRpcContacts = NULL;

    RpcTryExcept { ret = client_GetContacts(pNum, &ppRpcContacts); }
    RpcExcept(1)
    {
        ulCode = RpcExceptionCode();
        printf("RpcGetContacts exception 0x%lx = %ld\n", ulCode, ulCode);
    }
    RpcEndExcept;
    if (ret != 0) {
        printf("GetContacts Failed: %d\n", ret);
        return NULL;
    }

    return ppRpcContacts;
}

BSTR *RpcGetDbNames(int *pNum)
{
    int ret              = 0;
    unsigned long ulCode = 0;
    BSTR *pBstr          = NULL;

    RpcTryExcept { ret = client_GetDbNames(pNum, &pBstr); }
    RpcExcept(1)
    {
        ulCode = RpcExceptionCode();
        printf("RpcGetDbNames exception 0x%lx = %ld\n", ulCode, ulCode);
    }
    RpcEndExcept;
    if (ret != 0) {
        printf("RpcGetDbNames Failed: %d\n", ret);
        return NULL;
    }

    return pBstr;
}

PPRpcTables RpcGetDbTables(const wchar_t *db, int *pNum)
{
    int ret                 = 0;
    unsigned long ulCode    = 0;
    PPRpcTables ppRpcTables = NULL;

    RpcTryExcept { ret = client_GetDbTables(db, pNum, &ppRpcTables); }
    RpcExcept(1)
    {
        ulCode = RpcExceptionCode();
        printf("RpcGetDbTables exception 0x%lx = %ld\n", ulCode, ulCode);
    }
    RpcEndExcept;
    if (ret != 0) {
        printf("RpcGetDbTables Failed: %d\n", ret);
        return NULL;
    }

    return ppRpcTables;
}

int server_ReceiveMsg(RpcMessage_t rpcMsg)
{
    WxMessage_t msg;
    GetRpcMessage(&msg, rpcMsg);
    try {
        g_cbReceiveTextMsg(msg); // 调用接收消息回调
    } catch (...) {
        printf("callback error...\n");
    }

    return 0;
}
