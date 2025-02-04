#include "util.h"

#include <codecvt>
#include <locale>
#include <optional>
#include <strsafe.h>
#include <wchar.h>

#include "framework.h"
#include <Shlwapi.h>
#include <tlhelp32.h>

#include "log.hpp"

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "Version.lib")

namespace util
{

constexpr std::wstring_view WECHATEXE   = L"WeChat.exe";
constexpr std::string_view WECHATWINDLL = "WeChatWin.dll";

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

static DWORD get_wechat_pid()
{
    DWORD pid        = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
    while (Process32Next(hSnapshot, &pe32)) {
        if (pe32.szExeFile == WECHATEXE) {
            pid = pe32.th32ProcessID;
            break;
        }
    }
    CloseHandle(hSnapshot);
    return pid;
}

static std::optional<std::string> get_wechat_path()
{
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Tencent\\WeChat", 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        LOG_ERROR("无法打开注册表项");
        return std::nullopt;
    }

    char path[MAX_PATH] = { 0 };
    DWORD type          = REG_SZ;
    DWORD size          = sizeof(path);
    if (RegQueryValueExA(hKey, "InstallPath", nullptr, &type, reinterpret_cast<LPBYTE>(path), &size) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        LOG_ERROR("无法读取注册表中的 InstallPath");
        return std::nullopt;
    }
    RegCloseKey(hKey);

    PathAppendA(path, WECHATEXE);
    return std::string(path);
}

static std::optional<std::string> get_wechat_win_dll_path()
{
    auto wechat_path = get_wechat_path();
    if (!wechat_path) {
        return std::nullopt;
    }

    std::string dll_path = *wechat_path;
    PathRemoveFileSpecA(dll_path.data());
    PathAppendA(dll_path.data(), WECHATWINDLL);

    if (PathFileExistsA(dll_path.c_str())) {
        return dll_path;
    }

    // 微信从（大约）3.7开始，增加了一层版本目录: [3.7.0.29]
    PathRemoveFileSpecA(dll_path.data());
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA((dll_path + "\\*.*").c_str(), &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        return std::nullopt;
    }
    FindClose(hFind);

    std::string versioned_path = dll_path + "\\" + find_data.cFileName + WECHATWINDLL;
    return PathFileExistsA(versioned_path.c_str()) ? std::optional<std::string>(versioned_path) : std::nullopt;
}

static std::optional<std::string> get_file_version(const std::string &file_path)
{
    if (!PathFileExistsA(file_path.c_str())) {
        return std::nullopt;
    }

    DWORD dummy = 0;
    DWORD size  = GetFileVersionInfoSizeA(file_path.c_str(), &dummy);
    if (size == 0) {
        return std::nullopt;
    }

    std::vector<BYTE> buffer(size);
    if (!GetFileVersionInfoA(file_path.c_str(), 0, size, buffer.data())) {
        return std::nullopt;
    }

    VS_FIXEDFILEINFO *ver_info = nullptr;
    UINT ver_size              = 0;
    if (!VerQueryValueA(buffer.data(), "\\", reinterpret_cast<LPVOID *>(&ver_info), &ver_size)) {
        return std::nullopt;
    }

    return fmt::format("{}.{}.{}.{}", HIWORD(ver_info->dwFileVersionMS), LOWORD(ver_info->dwFileVersionMS),
                       HIWORD(ver_info->dwFileVersionLS), LOWORD(ver_info->dwFileVersionLS));
}

std::string get_wechat_version()
{
    auto dll_path = get_wechat_win_dll_path();
    if (!dll_path) {
        LOG_ERROR("无法获取 WeChatWin.dll 路径");
        return "";
    }

    auto version = get_file_version(*dll_path);
    if (!version) {
        LOG_ERROR("无法获取 WeChat 版本信息");
        return "";
    }

    return *version;
}

int open_wechat(DWORD &pid)
{
    pid = get_wechat_pid();
    if (pid != 0) {
        return ERROR_SUCCESS;
    }

    auto wechat_path = util::get_wechat_path();
    if (!wechat_path) {
        LOG_ERROR("获取 WeChat 安装路径失败");
        return ERROR_FILE_NOT_FOUND;
    }

    STARTUPINFOA si        = { sizeof(si) };
    PROCESS_INFORMATION pi = {};

    std::string command_line = *wechat_path;
    if (!CreateProcessA(nullptr, command_line.data(), nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, nullptr,
                        &si, &pi)) {
        return GetLastError();
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    pid = pi.dwProcessId;
    return ERROR_SUCCESS;
}

uint32_t get_memory_int_by_address(HANDLE hProcess, uint64_t addr)
{
    uint32_t value = 0;
    if (!addr || !hProcess) return value;

    ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(addr), &value, sizeof(value), nullptr);

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
