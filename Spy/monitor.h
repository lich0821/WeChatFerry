#pragma once

#include "framework.h"

int InitDLL(void);
DWORD WINAPI Monitor(HMODULE hModule);
int IsLogin();
