#include <atlstr.h>
#include <stdio.h>

#include "load_calls.h"
#include "monitor.h"
#include "receive_msg.h"
#include "util.h"

HANDLE g_hEvent          = NULL;
WxCalls_t g_WxCalls      = { 0 };
RpcMessage_t *g_pMsg     = NULL; // Find a palce to free
DWORD g_WeChatWinDllAddr = 0;

int InitDLL(void)
{
    wchar_t version[16] = { 0 };

    g_WeChatWinDllAddr = (DWORD)LoadLibrary(L"WeChatWin.dll"); //获取wechatWin模块地址
    if (g_WeChatWinDllAddr == 0) {
        MessageBox(NULL, L"获取wechatWin.dll模块地址失败", L"错误", 0);
        return -1;
    }

    if (!GetWeChatVersion(version)) { //获取微信版本
        MessageBox(NULL, L"获取微信版本失败", L"错误", 0);
        return -2;
    }

    if (LoadCalls(version, &g_WxCalls) != 0) { //加载微信版本对应的Call地址
        MessageBox(NULL, L"不支持当前版本", L"错误", 0);
        return -3;
    }

    g_pMsg   = new RpcMessage_t;
    g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    return 0;
}

DWORD WINAPI Monitor(HMODULE hModule)
{
    //ListenMessage();

    return TRUE;
}

int IsLogin(void)
{
    if (g_WeChatWinDllAddr == 0) {
        return 0;
    }

    return (int)GET_DWORD(g_WeChatWinDllAddr + g_WxCalls.login);
}
