#pragma once

#include <string>

#define WECHAREXE       L"WeChat.exe"
#define WECHATWINDLL    L"WeChatWin.dll"
#define WECHATSDKDLL    L"SDK.dll"
#define WECHATINJECTDLL L"Spy.dll"

#define GET_DWORD(addr)   ((DWORD) * (DWORD *)(addr))
#define GET_STRING(addr)  ((CHAR *)(addr))
#define GET_WSTRING(addr) ((WCHAR *)(*(DWORD *)(addr)))

int OpenWeChat(DWORD *pid);
int GetWeChatPath(wchar_t *path);
int GetWeChatWinDLLPath(wchar_t *path);
int GetWeChatVersion(wchar_t *version);
bool GetFileVersion(const wchar_t *filePath, wchar_t *version);
int GetWstringByAddress(DWORD address, wchar_t *buffer, DWORD buffer_size);
DWORD GetMemoryIntByAddress(HANDLE hProcess, DWORD address);
std::wstring GetUnicodeInfoByAddress(HANDLE hProcess, DWORD address);
