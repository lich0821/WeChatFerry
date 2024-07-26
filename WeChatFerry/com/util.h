#pragma once

#include <string>

#include "spy_types.h"

#define WECHAREXE       L"WeChat.exe"
#define WECHATWINDLL    L"WeChatWin.dll"
#define WCFSDKDLL       L"sdk.dll"
#define WCFSPYDLL       L"spy.dll"
#define WCFSPYDLL_DEBUG L"spy_debug.dll"

#define GET_UINT64(addr)         ((UINT64) * (UINT64 *)(addr))
#define GET_DWORD(addr)          ((DWORD) * (UINT64 *)(addr))
#define GET_QWORD(addr)          ((UINT64) * (UINT64 *)(addr))
#define GET_STRING(addr)         ((CHAR *)(*(UINT64 *)(addr)))
#define GET_WSTRING(addr)        ((WCHAR *)(*(UINT64 *)(addr)))
#define GET_STRING_FROM_P(addr)  ((CHAR *)(addr))
#define GET_WSTRING_FROM_P(addr) ((WCHAR *)(addr))

typedef struct PortPath {
    int port;
    char path[MAX_PATH];
} PortPath_t;

DWORD GetWeChatPid();
BOOL IsProcessX64(DWORD pid);
int OpenWeChat(DWORD *pid);
int GetWeChatVersion(wchar_t *version);
size_t GetWstringByAddress(UINT64 address, wchar_t *buffer, UINT64 buffer_size);
UINT32 GetMemoryIntByAddress(HANDLE hProcess, UINT64 address);
std::wstring GetUnicodeInfoByAddress(HANDLE hProcess, UINT64 address);
std::wstring String2Wstring(std::string s);
std::string Wstring2String(std::wstring ws);
std::string GB2312ToUtf8(const char *gb2312);
std::string GetStringByAddress(UINT64 address);
std::string GetStringByStrAddr(UINT64 addr);
std::string GetStringByWstrAddr(UINT64 addr);
void DbgMsg(const char *zcFormat, ...);
WxString *NewWxStringFromStr(const std::string &str);
WxString *NewWxStringFromWstr(const std::wstring &ws);
