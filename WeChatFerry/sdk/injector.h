#pragma once

#include "framework.h"

// 错误上报：非交互(默认)写 stderr + 返回码，交互场景(--gui)才弹模态框。
// 定义在 injector.cpp，sdk.cpp 与 injector.cpp 共用。
extern bool g_guiMode;
void ReportError(LPCWSTR message, LPCWSTR title);

HANDLE InjectDll(DWORD pid, LPCWSTR dllPath, HMODULE *injectedBase);
bool EjectDll(HANDLE process, HMODULE dllBase);
bool CallDllFunc(HANDLE process, LPCWSTR dllPath, HMODULE dllBase, LPCSTR funcName, LPVOID parameter, DWORD *ret);
bool CallDllFuncEx(HANDLE process, LPCWSTR dllPath, HMODULE dllBase, LPCSTR funcName, LPVOID parameter, size_t sz,
                   DWORD *ret);
