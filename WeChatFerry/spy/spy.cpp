#include <filesystem>

#include "load_calls.h"
#include "log.h"
#include "rpc_server.h"
#include "spy.h"
#include "util.h"

WxCalls_t g_WxCalls      = { 0 };
DWORD g_WeChatWinDllAddr = 0;

void InitSpy(LPVOID args)
{
    wchar_t version[16] = { 0 };
    PortPath_t *pp      = (PortPath_t *)args;
    int port            = pp->port;
    std::string path(pp->path);

    InitLogger(path);
    g_WeChatWinDllAddr = (DWORD)GetModuleHandle(L"WeChatWin.dll"); // 获取wechatWin模块地址
    if (g_WeChatWinDllAddr == 0) {
        LOG_ERROR("获取wechatWin.dll模块地址失败");
        return;
    }

    if (!GetWeChatVersion(version)) { // 获取微信版本
        LOG_ERROR("获取微信版本失败");
        return;
    }
    LOG_DEBUG("WeChat version: {}", Wstring2String(version).c_str());
    if (LoadCalls(version, &g_WxCalls) != 0) { // 加载微信版本对应的Call地址
        LOG_ERROR("不支持当前版本");
        MessageBox(NULL, L"不支持当前版本", L"错误", 0);
        return;
    }

    RpcStartServer(port);
}

void CleanupSpy() { RpcStopServer(); }

int IsLogin(void) { return (int)GET_DWORD(g_WeChatWinDllAddr + g_WxCalls.login); }
