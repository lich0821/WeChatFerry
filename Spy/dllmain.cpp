#include <stdlib.h>

#include "monitor.h"
#include "rpc_server.h"

extern HANDLE g_hEvent;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            // MessageBox(NULL, L"RpcStartServer", L"Hey", 0);
            if (InitDLL() != 0) {
                // Exit
                FreeLibraryAndExitThread(hModule, 0);
            }
            HANDLE rpcThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RpcStartServer, hModule, NULL, 0);
            if (rpcThread != 0) {
                CloseHandle(rpcThread);
            }
            g_hEvent       = CreateEvent(NULL, TRUE, FALSE, NULL); // 创建消息句柄
            HANDLE mThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Monitor, hModule, NULL, 0);
            if (mThread != 0) {
                CloseHandle(mThread);
            }
            break;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH: {
            RpcStopServer();
            break;
        }
    }
    return TRUE;
}
