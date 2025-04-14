#pragma once

#include "framework.h"

HANDLE InjectDll(DWORD pid, LPCWSTR dllPath, HMODULE *injectedBase);
bool EjectDll(HANDLE process, HMODULE dllBase);
bool CallDllFunc(HANDLE process, LPCWSTR dllPath, HMODULE dllBase, LPCSTR funcName, DWORD *ret);
bool CallDllFuncEx(HANDLE process, LPCWSTR dllPath, HMODULE dllBase, LPCSTR funcName, LPVOID parameter, size_t sz,
                   DWORD *ret);
