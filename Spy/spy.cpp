#include "load_calls.h"
#include "rpc_server.h"
#include "spy.h"
#include "util.h"

HANDLE g_hEvent          = NULL;
BOOL g_rpcKeepAlive      = false;
WxCalls_t g_WxCalls      = { 0 };
DWORD g_WeChatWinDllAddr = 0;

void InitSpy()
{
    wchar_t version[16] = { 0 };

    g_WeChatWinDllAddr = (DWORD)GetModuleHandle(L"WeChatWin.dll"); //获取wechatWin模块地址
    if (g_WeChatWinDllAddr == 0) {
        MessageBox(NULL, L"获取wechatWin.dll模块地址失败", L"错误", 0);
        return;
    }

    if (!GetWeChatVersion(version)) { //获取微信版本
        MessageBox(NULL, L"获取微信版本失败", L"错误", 0);
        return;
    }

    if (LoadCalls(version, &g_WxCalls) != 0) { //加载微信版本对应的Call地址
        MessageBox(NULL, L"不支持当前版本", L"错误", 0);
        return;
    }

    g_hEvent         = CreateEvent(NULL, TRUE, FALSE, NULL);
    HANDLE rpcThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RpcStartServer, NULL, NULL, 0);
    if (rpcThread != 0) {
        CloseHandle(rpcThread);
    }
}

void DestroySpy(HMODULE hModule)
{
    RpcStopServer();
    FreeLibraryAndExitThread(hModule, 0);
}

int IsLogin(void) { return (int)GET_DWORD(g_WeChatWinDllAddr + g_WxCalls.login); }

wstring GetSelfWxid() { return String2Wstring(GET_STRING(g_WeChatWinDllAddr + g_WxCalls.ui.wxid)); }
