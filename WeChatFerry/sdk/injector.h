#pragma once

#include <string>

#include "framework.h"

HANDLE inject_dll(DWORD pid, const std::string &dll_path, HMODULE *injected_base);
bool eject_dll(HANDLE process, HMODULE dll_base);
bool call_dll_func(HANDLE process, const std::string &dll_path, HMODULE dll_base, const std::string &func, DWORD *ret);
bool call_dll_func_ex(HANDLE process, const std::string &dll_path, HMODULE dll_base, const std::string &func,
                      LPVOID parameter, size_t size, DWORD *ret);
