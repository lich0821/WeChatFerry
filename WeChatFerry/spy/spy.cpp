#include "spy.h"

#include <filesystem>

#include "log.hpp"
#include "util.h"
#include "offsets.h"
#include "rpc_server.h"

// 全局变量
uint32_t g_WeChatWinDllAddr = 0;

namespace Spy
{

void init(void *args)
{
    wchar_t version[16] = { 0 };
    PortPath_t *pp      = (PortPath_t *)args;
    int port            = pp->port;
    std::string path(pp->path);

    init_logger(path);
    g_WeChatWinDllAddr = (uint32_t)GetModuleHandle(L"WeChatWin.dll");
    if (g_WeChatWinDllAddr == 0) {
        LOG_ERROR("获取wechatWin.dll模块地址失败");
        return;
    }

    if (!util::get_wechat_version(version)) {
        LOG_ERROR("获取微信版本失败");
        return;
    }

    std::wstring ws_version(version);
    LOG_DEBUG("WeChat version: {}", util::w2s(ws_version).c_str());

    if (ws_version != std::wstring(SUPPORT_VERSION.begin(), SUPPORT_VERSION.end())) {
        LOG_ERROR("不支持当前版本");
        MessageBox(NULL, L"不支持当前版本", L"错误", 0);
        return;
    }

    RpcStartServer(port);
}

void cleanup()
{
    RpcStopServer();
}

bool is_logged_in()
{
    return g_WeChatWinDllAddr && util::get_dword(g_WeChatWinDllAddr + Offsets::Account::SERVICE) != 0;
}

} // namespace Spy

// C 接口实现
extern "C" {

void InitSpy(void *args)
{
    Spy::init(args);
}

void CleanupSpy()
{
    Spy::cleanup();
}

int IsLogin(void)
{
    return Spy::is_logged_in() ? 1 : 0;
}

}
