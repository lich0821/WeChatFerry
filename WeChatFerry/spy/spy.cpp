#include "spy.h"

#include <filesystem>
#include <string_view>

#include "log.hpp"
#include "rpc_server.h"
#include "util.h"

constexpr std::string_view SUPPORT_VERSION = "3.9.11.25";

UINT64 g_WeChatWinDllAddr = 0;

void InitSpy(LPVOID args)
{
    auto *pp = static_cast<util::PortPath *>(args);

    Log::InitLogger(pp->path);
    if (auto dll_addr = GetModuleHandle(L"WeChatWin.dll")) {
        g_WeChatWinDllAddr = reinterpret_cast<UINT64>(dll_addr);
    } else {
        LOG_ERROR("获取 WeChatWin.dll 模块地址失败");
        return; // TODO: 退出进程，避免后面操作失败
    }

    std::string version = util::get_wechat_version();
    std::string msg     = fmt::format("WCF 支持版本: {}，当前版本: {}", SUPPORT_VERSION, version);
    if (version != SUPPORT_VERSION) {
        LOG_ERROR(msg);
        MessageBoxA(NULL, msg.c_str(), "错误", MB_ICONERROR);
        return;
    }

    LOG_INFO(msg);
    RpcStartServer(pp->port);
}

void CleanupSpy()
{
    LOG_DEBUG("CleanupSpy");
    RpcStopServer();
}
