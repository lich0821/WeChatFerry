#pragma once

#include <string>

#include "rpc_h.h"
#include "sdk.h"

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
BSTR GetBstrByAddress(DWORD address);
void GetRpcMessage(WxMessage_t *wxMsg, RpcMessage_t rpcMsg);
DWORD GetMemoryIntByAddress(HANDLE hProcess, DWORD address);
std::wstring GetWstringFromBstr(BSTR p);
BSTR GetBstrFromString(const char *str);
BSTR GetBstrFromWstring(std::wstring ws);
std::wstring GetUnicodeInfoByAddress(HANDLE hProcess, DWORD address);
