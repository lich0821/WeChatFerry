#include "load_calls.h"
#include "receive_msg.h"
#include "rpc_server.h"
#include "spy.h"
#include "util.h"

HANDLE g_hEvent          = NULL;
WxCalls_t g_WxCalls      = { 0 };
DWORD g_WeChatWinDllAddr = 0;

DWORD WINAPI Monitor(HMODULE hModule)
{
    ListenMessage();

    return TRUE;
}

void InitSpy(HMODULE hModule)
{
    wchar_t version[16] = { 0 };

    g_WeChatWinDllAddr = (DWORD)LoadLibrary(L"WeChatWin.dll"); //获取wechatWin模块地址
    if (g_WeChatWinDllAddr == 0) {
        MessageBox(NULL, L"获取wechatWin.dll模块地址失败", L"错误", 0);
        FreeLibraryAndExitThread(hModule, 0);
    }

    if (!GetWeChatVersion(version)) { //获取微信版本
        MessageBox(NULL, L"获取微信版本失败", L"错误", 0);
        FreeLibraryAndExitThread(hModule, 0);
    }

    if (LoadCalls(version, &g_WxCalls) != 0) { //加载微信版本对应的Call地址
        MessageBox(NULL, L"不支持当前版本", L"错误", 0);
        FreeLibraryAndExitThread(hModule, 0);
    }

    g_hEvent         = CreateEvent(NULL, TRUE, FALSE, NULL);
    HANDLE rpcThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RpcStartServer, hModule, NULL, 0);
    if (rpcThread != 0) {
        CloseHandle(rpcThread);
    }

    HANDLE mThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Monitor, hModule, NULL, 0);
    if (mThread != 0) {
        CloseHandle(mThread);
    }
}

void DestroySpy() { RpcStopServer(); }

int IsLogin(void)
{
    if (g_WeChatWinDllAddr == 0) {
        return 0;
    }

    return (int)GET_DWORD(g_WeChatWinDllAddr + g_WxCalls.login);
}
