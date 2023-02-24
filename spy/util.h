#pragma once

#include <string>

#define WECHAREXE             L"WeChat.exe"
#define WECHATWINDLL          L"WeChatWin.dll"
#define WECHATSDKDLL          L"sdk.dll"
#define WECHATINJECTDLL       L"spy.dll"
#define WECHATINJECTDLL_DEBUG L"spy_debug.dll"

#define GET_DWORD(addr)   ((DWORD) * (DWORD *)(addr))
#define GET_STRING(addr)  ((CHAR *)(*(DWORD *)(addr)))
#define GET_WSTRING(addr) ((WCHAR *)(*(DWORD *)(addr)))

int OpenWeChat(DWORD *pid);
int GetWeChatVersion(wchar_t *version);
int GetWstringByAddress(DWORD address, wchar_t *buffer, DWORD buffer_size);
DWORD GetMemoryIntByAddress(HANDLE hProcess, DWORD address);
std::wstring GetUnicodeInfoByAddress(HANDLE hProcess, DWORD address);
std::wstring String2Wstring(std::string s);
std::string Wstring2String(std::wstring ws);
std::string GetStringByAddress(DWORD address);
