#include "sdk.h"
#include "util.h"
#include "rpc_client.h"

#pragma comment(lib, "Rpcrt4.lib")

static RPC_WSTR pszStringBinding = NULL;
extern std::function<int(WxMessage_t)> g_cbReceiveTextMsg;

RPC_STATUS RpcConnectServer()
{
    RPC_STATUS status = 0;
    // Creates a string binding handle.
    status = RpcStringBindingCompose(NULL,                                             // UUID to bind to
                                     reinterpret_cast<RPC_WSTR>((RPC_WSTR)L"ncalrpc"), // Use TCP/IP protocol
                                     NULL,                                             // TCP/IP network address to use
                                     reinterpret_cast<RPC_WSTR>((RPC_WSTR)L"tmp_endpoint"), // TCP/IP port to use
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
    if (status)
        return status;

    // Releases binding handle resources and disconnects from the server
    status = RpcBindingFree(&hSpyBinding);

    return status;
}

unsigned int __stdcall RpcSetTextMsgCb(void *p)
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
        printf("rpcWxSetTextMsgCb exception 0x%lx = %ld\n", ulCode, ulCode);
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
        printf("rpcIsLogin exception 0x%lx = %ld\n", ulCode, ulCode);
    }
    RpcEndExcept;

    return loginFlag;
}

int RpcSendTextMsg(const wchar_t *wxid, const wchar_t *at_wxid, const wchar_t *msg)
{
    int ret              = 0;
    unsigned long ulCode = 0;

    RpcTryExcept { ret = client_SendTextMsg(wxid, at_wxid, msg); }
    RpcExcept(1)
    {
        ulCode = RpcExceptionCode();
        printf("rpcWxSendTextMsg exception 0x%lx = %ld\n", ulCode, ulCode);
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
        printf("rpcWxSendImageMsg exception 0x%lx = %ld\n", ulCode, ulCode);
    }
    RpcEndExcept;

    return ret;
}

PPRpcIntBstrPair_t RpcGetMsgTypes(int *pNum)
{
    int ret = 0;
    unsigned long ulCode = 0;
    PPRpcIntBstrPair_t ppRpcMsgTypes = NULL;

    RpcTryExcept{ ret = client_GetMsgTypes(pNum, &ppRpcMsgTypes); }
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
