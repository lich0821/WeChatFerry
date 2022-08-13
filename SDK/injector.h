#pragma once

#include "framework.h"

int InjectDll(DWORD pid, const WCHAR* dllPath);
int EjectDll(DWORD pid, const WCHAR* dllPath);
