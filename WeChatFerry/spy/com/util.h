#pragma once

#include <cstdint>
#include <string>

#include "framework.h"

#define WECHAREXE             L"WeChat.exe"
#define WECHATWINDLL          L"WeChatWin.dll"
#define WECHATSDKDLL          L"sdk.dll"
#define WECHATINJECTDLL       L"spy.dll"
#define WECHATINJECTDLL_DEBUG L"spy_debug.dll"

typedef struct PortPath {
    int port;
    char path[MAX_PATH];
} PortPath_t;

namespace util
{

// Inline 函数替代宏，提供类型安全
inline uint32_t get_dword(uint32_t addr)
{
    return addr ? *reinterpret_cast<uint32_t *>(addr) : 0;
}

inline uint64_t get_qword(uint64_t addr)
{
    return addr ? *reinterpret_cast<uint64_t *>(addr) : 0;
}

inline const char *get_string(uint32_t addr)
{
    return addr ? *reinterpret_cast<const char **>(addr) : nullptr;
}

inline const wchar_t *get_wstring(uint32_t addr)
{
    return addr ? *reinterpret_cast<const wchar_t **>(addr) : nullptr;
}

inline const char *get_p_string(uint32_t addr)
{
    return addr ? reinterpret_cast<const char *>(addr) : nullptr;
}

inline const wchar_t *get_p_wstring(uint32_t addr)
{
    return addr ? reinterpret_cast<const wchar_t *>(addr) : nullptr;
}

inline std::string get_pp_string(uint32_t addr)
{
    if (!addr)
        return "";
    const char *ptr = *reinterpret_cast<const char **>(addr);
    return (ptr && *ptr) ? std::string(ptr) : "";
}

// 函数声明（使用 snake_case 命名）
uint32_t get_wechat_pid();
int open_wechat(uint32_t *pid);
int get_wechat_version(wchar_t *version);
int get_wstring_by_address(uint32_t address, wchar_t *buffer, uint32_t buffer_size);
uint32_t get_memory_int_by_address(void *hProcess, uint32_t address);
std::wstring get_unicode_info_by_address(void *hProcess, uint32_t address);
std::wstring s2w(const std::string &s);
std::string w2s(const std::wstring &ws);
std::string gb2312_to_utf8(const char *gb2312);
std::string get_string_by_address(uint32_t address);
std::string get_string_by_str_addr(uint32_t addr);
std::string get_string_by_wstr_addr(uint32_t addr);
void dbg_msg(const char *format, ...);

} // namespace util
