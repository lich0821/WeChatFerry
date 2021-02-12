// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "framework.h"
#include <rpc.h>

extern RPC_STATUS RpcConnectServer();
extern RPC_STATUS RpcDisconnectServer();

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH: {
            RpcDisconnectServer();
            break;
        }
    }
    return TRUE;
}
