#pragma once

#include <atomic>
#include <cstdint>
#include <string_view>

#include "framework.h"

namespace Spy
{

constexpr std::string_view SUPPORT_VERSION = "3.9.12.56";

void init(void *args);
void cleanup();
bool is_logged_in();

} // namespace Spy

// WeChatWin.dll 基址（运行时由 InitSpy 填入）
extern uint32_t g_WeChatWinDllAddr;

// 供 sdk.dll 通过 CallDllFuncEx 调用的 C 接口
extern "C" {
void InitSpy(void *args);
void CleanupSpy();
int IsLogin(void);
}
