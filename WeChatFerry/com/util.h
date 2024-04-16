#pragma once

#include <string>

#define WECHAREXE             L"WeChat.exe"
#define WECHATWINDLL          L"WeChatWin.dll"
#define WECHATSDKDLL          L"sdk.dll"
#define WECHATINJECTDLL       L"spy.dll"
#define WECHATINJECTDLL_DEBUG L"spy_debug.dll"

#define GET_DWORD(addr)          ((DWORD) * (DWORD *)(addr))
#define GET_QWORD(addr)          ((uint64_t) * (uint64_t *)(addr))
#define GET_STRING(addr)         ((CHAR *)(*(DWORD *)(addr)))
#define GET_WSTRING(addr)        ((WCHAR *)(*(DWORD *)(addr)))
#define GET_STRING_FROM_P(addr)  ((CHAR *)(addr))
#define GET_WSTRING_FROM_P(addr) ((WCHAR *)(addr))

typedef struct PortPath {
    int port;
    char path[MAX_PATH];
} PortPath_t;

DWORD GetWeChatPid();
int OpenWeChat(DWORD *pid);
int GetWeChatVersion(wchar_t *version);
int GetWstringByAddress(DWORD address, wchar_t *buffer, DWORD buffer_size);
DWORD GetMemoryIntByAddress(HANDLE hProcess, DWORD address);
std::wstring GetUnicodeInfoByAddress(HANDLE hProcess, DWORD address);
std::wstring String2Wstring(std::string s);
std::string Wstring2String(std::wstring ws);
std::string GB2312ToUtf8(const char *gb2312);
std::string GetStringByAddress(DWORD address);
std::string GetStringByStrAddr(DWORD addr);
std::string GetStringByWstrAddr(DWORD addr);
void DbgMsg(const char *zcFormat, ...);
