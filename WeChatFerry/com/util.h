#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "spy_types.h"

namespace util
{
struct PortPath {
    int port;
    char path[MAX_PATH];
};

DWORD get_wechat_pid();
int open_wechat(DWORD &pid);
std::string get_wechat_version();
uint32_t get_memory_int_by_address(HANDLE hProcess, uint64_t addr);
std::wstring get_unicode_info_by_address(HANDLE hProcess, uint64_t addr);
std::wstring s2w(const std::string &s);
std::string w2s(const std::wstring &ws);
std::string gb2312_to_utf8(const char *gb2312);
void dbg_msg(const char *format, ...);

inline DWORD get_dword(uint64_t addr) { return addr ? *reinterpret_cast<DWORD *>(addr) : 0; }
inline QWORD get_qword(uint64_t addr) { return addr ? *reinterpret_cast<QWORD *>(addr) : 0; }
inline uint64_t get_uint64(uint64_t addr) { return addr ? *reinterpret_cast<uint64_t *>(addr) : 0; }
inline std::string get_p_string(uint64_t addr) { return addr ? std::string(reinterpret_cast<const char *>(addr)) : ""; }
inline std::string get_p_string(uint64_t addr, size_t len)
{
    return addr ? std::string(reinterpret_cast<const char *>(addr), len) : "";
}
inline std::wstring get_p_wstring(uint64_t addr)
{
    return addr ? std::wstring(reinterpret_cast<const wchar_t *>(addr)) : L"";
}
inline std::wstring get_p_wstring(uint64_t addr, size_t len)
{
    return addr ? std::wstring(reinterpret_cast<const wchar_t *>(addr), len) : L"";
}
inline std::string get_pp_string(uint64_t addr)
{
    if (!addr) return "";

    const char *ptr = *reinterpret_cast<const char **>(addr);
    return (ptr && *ptr) ? std::string(ptr) : "";
}
inline std::wstring get_pp_wstring(uint64_t addr)
{
    if (!addr) return L"";

    const wchar_t *ptr = *reinterpret_cast<const wchar_t **>(addr);
    return (ptr && *ptr) ? std::wstring(ptr) : L"";
}
inline std::string get_pp_len_string(uint64_t addr)
{
    size_t len = get_dword(addr + 8);
    return (addr && len) ? std::string(*reinterpret_cast<const char **>(addr), len) : "";
}
inline std::wstring get_pp_len_wstring(uint64_t addr)
{
    size_t len = get_dword(addr + 8);
    return (addr && len) ? std::wstring(*reinterpret_cast<const wchar_t **>(addr), len) : L"";
}
inline std::string get_str_by_wstr_addr(uint64_t addr) { return w2s(get_pp_len_wstring(addr)); }
inline void *AllocFromHeap(size_t size) { return HeapAlloc(GetProcessHeap(), 8, size); }
inline void FreeBuffer(void *buffer)
{
    if (buffer) HeapFree(GetProcessHeap(), 8, buffer);
}
inline int MsgBox(HWND hWnd, const std::string &text, const std::string &caption = "WCF", UINT uType = MB_OK)
{
    std::wstring wText    = s2w(text);
    std::wstring wCaption = s2w(caption);
    return MessageBoxW(nullptr, wText.c_str(), wCaption.c_str(), uType);
}

template <typename T> static T *AllocBuffer(size_t count)
{
    return reinterpret_cast<T *>(HeapAlloc(GetProcessHeap(), 8, sizeof(T) * count));
}

template <typename T> struct WxStringHolder {
    std::wstring ws;
    WxString wx;
    explicit WxStringHolder(const T &str) : ws(util::s2w(str)), wx(ws) { }
};

template <typename StringT = std::wstring> struct AtWxidSplitResult {
    std::vector<StringT> wxids;
    std::vector<WxString> wxWxids;
};

WxString *CreateWxString(const std::string &s);
void FreeWxString(WxString *wxStr);
AtWxidSplitResult<> parse_wxids(const std::string &atWxids);

std::unique_ptr<WxString> new_wx_string(const char *str);
std::unique_ptr<WxString> new_wx_string(const wchar_t *wstr);
std::unique_ptr<WxString> new_wx_string(const std::string &str);
std::unique_ptr<WxString> new_wx_string(const std::wstring &wstr);

} // namespace util
