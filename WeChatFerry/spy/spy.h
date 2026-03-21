#pragma once

#include <atomic>
#include <cstdint>
#include <string_view>

#include "framework.h"

namespace Spy
{

constexpr std::string_view SUPPORT_VERSION = "3.9.2.23";

// 初始化和清理
void init(void *args);
void cleanup();

// 登录状态检查
bool is_logged_in();

} // namespace Spy

// 全局变量：WeChatWin.dll 基址（保留以兼容现有代码）
extern uint32_t g_WeChatWinDllAddr;

// 保留 C 接口以兼容
extern "C" {
void InitSpy(void *args);
void CleanupSpy();
int IsLogin(void);
}
