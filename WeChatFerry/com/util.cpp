#include "util.h"

#include <codecvt>
#include <locale>
#include <memory>
#include <string.h>
#include <strsafe.h>
#include <tlhelp32.h>
#include <vector>
#include <wchar.h>

#include "framework.h"

#include "log.hpp"

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "Version.lib")

namespace util
{

std::wstring s2w(const std::string &s)
{
    if (s.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring ws(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), &ws[0], size_needed);
    return ws;
}

std::string w2s(const std::wstring &ws)
{
    if (ws.empty()) return std::string();
    int size_needed
        = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), static_cast<int>(ws.size()), nullptr, 0, nullptr, nullptr);
    std::string s(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), static_cast<int>(ws.size()), &s[0], size_needed, nullptr, nullptr);
    return s;
}

std::string gb2312_to_utf8(const char *gb2312)
{
    if (!gb2312) return "";

    int size_needed = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, nullptr, 0);
    std::wstring ws(size_needed, 0);
    MultiByteToWideChar(CP_ACP, 0, gb2312, -1, &ws[0], size_needed);

    size_needed = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &s[0], size_needed, nullptr, nullptr);

    return s;
}

DWORD get_wechat_pid()
{
    DWORD pid        = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
    while (Process32Next(hSnapshot, &pe32)) {
        if (std::wstring(pe32.szExeFile) == WECHATEXE) {
            pid = pe32.th32ProcessID;
            break;
        }
    }
    CloseHandle(hSnapshot);
    return pid;
}

int open_wechat(DWORD *pid)
{
    *pid = get_wechat_pid();
    if (*pid) return ERROR_SUCCESS;

    WCHAR path[MAX_PATH] = { 0 };
    if (GetModuleFileNameW(nullptr, path, MAX_PATH) == 0) {
        return GetLastError();
    }

    STARTUPINFO si         = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    if (!CreateProcessW(nullptr, path, nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi)) {
        return GetLastError();
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    *pid = pi.dwProcessId;
    return ERROR_SUCCESS;
}

static std::optional<std::string> get_wechat_win_dll_path()
{
    char path[MAX_PATH] = { 0 };
    if (GetWeChatPath(path) != ERROR_SUCCESS) {
        return std::nullopt;
    }

    PathRemoveFileSpecA(path);
    PathAppendA(path, WECHATWINDLL);

    if (!PathFileExistsA(path)) {
        // 微信 3.7+ 版本增加了一层目录
        PathRemoveFileSpecA(path);
        _finddata_t findData;
        std::string dir = std::string(path) + "\\[*.*";
        intptr_t handle = _findfirst(dir.c_str(), &findData);
        if (handle == -1) {
            return std::nullopt;
        }
        _findclose(handle);

        std::string dllPath = std::string(path) + "\\" + findData.name + "\\" + WECHATWINDLL;
        return dllPath;
    }

    return std::string(path);
}

static std::optional<std::string> get_file_version(const std::string &filePath)
{
    if (filePath.empty() || !PathFileExistsA(filePath.c_str())) {
        return std::nullopt;
    }

    DWORD handle = 0;
    DWORD size   = GetFileVersionInfoSizeA(filePath.c_str(), &handle);
    if (size == 0) {
        return std::nullopt;
    }

    std::vector<BYTE> data(size);
    if (!GetFileVersionInfoA(filePath.c_str(), 0, size, data.data())) {
        return std::nullopt;
    }

    VS_FIXEDFILEINFO *verInfo = nullptr;
    UINT len                  = 0;
    if (!VerQueryValueA(data.data(), "\\", reinterpret_cast<void **>(&verInfo), &len) || len == 0) {
        return std::nullopt;
    }

    char version[32];
    StringCbPrintfA(version, sizeof(version), "%d.%d.%d.%d", HIWORD(verInfo->dwFileVersionMS),
                    LOWORD(verInfo->dwFileVersionMS), HIWORD(verInfo->dwFileVersionLS),
                    LOWORD(verInfo->dwFileVersionLS));

    return std::string(version);
}

std::string get_wechat_version()
{
    std::string version = "";

    auto dllPath = get_wechat_win_dll_path();
    if (!dllPath) {
        return version;
    }

    version = get_file_version(*dllPath);
    return version ? version : "";
}

uint32_t get_memory_int_by_address(HANDLE hProcess, uint64_t addr)
{
    uint32_t value = 0;
    if (!addr || !hProcess) return value;

    unsigned char data[4] = { 0 };
    if (ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(addr), data, sizeof(data), nullptr)) {
        value = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    }

    return value;
}

std::wstring get_unicode_info_by_address(HANDLE hProcess, uint64_t address)
{
    if (!hProcess || !address) return L"";

    uint64_t str_address = get_memory_int_by_address(hProcess, address);
    uint64_t str_len     = get_memory_int_by_address(hProcess, address + 0x4);
    if (str_len > 500) return L"";

    wchar_t cValue[500] = { 0 };
    if (ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(str_address), cValue, (str_len + 1) * sizeof(wchar_t),
                          nullptr)) {
        return std::wstring(cValue);
    }

    return L"";
}

void dbg_msg(const char *format, ...)
{
    if (!format) return;

    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    std::vector<char> buffer(len + 1);
    vsnprintf(buffer.data(), buffer.size(), format, args);
    va_end(args);

    OutputDebugStringA(buffer.data());
}

std::unique_ptr<WxString> new_wx_string(const char *str)
{
    return new_wx_string(str ? std::string(str) : std::string());
}

std::unique_ptr<WxString> new_wx_string(const std::string &str) { return std::make_unique<WxString>(s2w(str)); }

std::unique_ptr<WxString> new_wx_string(const wchar_t *wstr)
{
    return new_wx_string(wstr ? std::wstring(wstr) : std::wstring());
}

std::unique_ptr<WxString> new_wx_string(const std::wstring &wstr) { return std::make_unique<WxString>(wstr); }

std::vector<WxString> parse_wxids(const std::string &wxids)
{
    std::vector<WxString> wx_members;
    std::wstringstream wss(s2w(wxids));
    std::wstring wstr;
    while (getline(wss, wstr, L',')) {
        wx_members.emplace_back(wstr);
    }
    return wx_members;
}

} // namespace util
