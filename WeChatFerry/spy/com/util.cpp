#include "util.h"

#include "Shlwapi.h"
#include "framework.h"
#include <codecvt>
#include <locale>
#include <string.h>
#include <strsafe.h>
#include <tlhelp32.h>
#include <vector>
#include <wchar.h>

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "Version.lib")

namespace util
{

std::wstring s2w(const std::string &s)
{
    if (s.empty())
        return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &s[0], (int)s.size(), NULL, 0);
    std::wstring ws(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &s[0], (int)s.size(), &ws[0], size_needed);
    return ws;
}

std::string w2s(const std::wstring &ws)
{
    if (ws.empty())
        return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.size(), NULL, 0, NULL, NULL);
    std::string s(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.size(), &s[0], size_needed, NULL, NULL);
    return s;
}

std::string gb2312_to_utf8(const char *gb2312)
{
    int size_needed = 0;

    size_needed = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
    std::wstring ws(size_needed, 0);
    MultiByteToWideChar(CP_ACP, 0, gb2312, -1, &ws[0], size_needed);

    size_needed = WideCharToMultiByte(CP_UTF8, 0, &ws[0], -1, NULL, 0, NULL, NULL);
    std::string s(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &ws[0], -1, &s[0], size_needed, NULL, NULL);

    return s;
}

static int get_wechat_path(wchar_t *path)
{
    int ret   = -1;
    HKEY hKey = NULL;
    // HKEY_CURRENT_USER\Software\Tencent\WeChat InstallPath = xx
    if (ERROR_SUCCESS != RegOpenKey(HKEY_CURRENT_USER, L"Software\\Tencent\\WeChat", &hKey)) {
        ret = GetLastError();
        return ret;
    }

    DWORD Type   = REG_SZ;
    DWORD cbData = MAX_PATH * sizeof(WCHAR);
    if (ERROR_SUCCESS != RegQueryValueEx(hKey, L"InstallPath", 0, &Type, (LPBYTE)path, &cbData)) {
        ret = GetLastError();
        goto __exit;
    }

    if (path != NULL) {
        PathAppend(path, WECHAREXE);
    }

__exit:
    if (hKey) {
        RegCloseKey(hKey);
    }

    return ERROR_SUCCESS;
}

static int get_wechat_win_dll_path(wchar_t *path)
{
    int ret = get_wechat_path(path);
    if (ret != ERROR_SUCCESS) {
        return ret;
    }

    PathRemoveFileSpecW(path);
    PathAppendW(path, WECHATWINDLL);
    if (!PathFileExists(path)) {
        // 微信从（大约）3.7开始，增加了一层版本目录: [3.7.0.29]
        PathRemoveFileSpec(path);
        _wfinddata_t findData;
        std::wstring dir     = std::wstring(path) + L"\\[*.*";
        intptr_t handle = _wfindfirst(dir.c_str(), &findData);
        if (handle == -1) { // 检查是否成功
            return -1;
        }
        std::wstring dllPath = std::wstring(path) + L"\\" + findData.name;
        wcscpy_s(path, MAX_PATH, dllPath.c_str());
        PathAppend(path, WECHATWINDLL);
    }

    return ret;
}

static bool get_file_version(const wchar_t *filePath, wchar_t *version)
{
    if (wcslen(filePath) > 0 && PathFileExists(filePath)) {
        VS_FIXEDFILEINFO *pVerInfo = NULL;
        DWORD dwTemp, dwSize;
        BYTE *pData = NULL;
        UINT uLen;

        dwSize = GetFileVersionInfoSize(filePath, &dwTemp);
        if (dwSize == 0) {
            return false;
        }

        pData = new BYTE[dwSize + 1];
        if (pData == NULL) {
            return false;
        }

        if (!GetFileVersionInfo(filePath, 0, dwSize, pData)) {
            delete[] pData;
            return false;
        }

        if (!VerQueryValue(pData, TEXT("\\"), (void **)&pVerInfo, &uLen)) {
            delete[] pData;
            return false;
        }

        DWORD verMS    = pVerInfo->dwFileVersionMS;
        DWORD verLS    = pVerInfo->dwFileVersionLS;
        DWORD major    = HIWORD(verMS);
        DWORD minor    = LOWORD(verMS);
        DWORD build    = HIWORD(verLS);
        DWORD revision = LOWORD(verLS);
        delete[] pData;

        StringCbPrintf(version, 0x20, TEXT("%d.%d.%d.%d"), major, minor, build, revision);

        return true;
    }

    return false;
}

int get_wechat_version(wchar_t *version)
{
    WCHAR Path[MAX_PATH] = { 0 };

    int ret = get_wechat_win_dll_path(Path);
    if (ret != ERROR_SUCCESS) {
        return ret;
    }

    ret = get_file_version(Path, version);

    return ret;
}

uint32_t get_wechat_pid()
{
    uint32_t pid           = 0;
    HANDLE hSnapshot    = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
    while (Process32Next(hSnapshot, &pe32)) {
        std::wstring strProcess = pe32.szExeFile;
        if (strProcess == WECHAREXE) {
            pid = pe32.th32ProcessID;
            break;
        }
    }
    CloseHandle(hSnapshot);
    return pid;
}

int open_wechat(uint32_t *pid)
{
    *pid = get_wechat_pid();
    if (*pid) {
        return ERROR_SUCCESS;
    }

    int ret                = -1;
    STARTUPINFO si         = { sizeof(si) };
    WCHAR Path[MAX_PATH]   = { 0 };
    PROCESS_INFORMATION pi = { 0 };

    ret = get_wechat_path(Path);
    if (ERROR_SUCCESS != ret) {
        return ret;
    }

    if (!CreateProcess(NULL, Path, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        return GetLastError();
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    *pid = pi.dwProcessId;

    return ERROR_SUCCESS;
}

int get_wstring_by_address(uint32_t address, wchar_t *buffer, uint32_t buffer_size)
{
    uint32_t strLength = get_dword(address + 4);
    if (strLength == 0) {
        return 0;
    } else if (strLength > buffer_size) {
        strLength = buffer_size - 1;
    }

    wmemcpy_s(buffer, strLength + 1, get_wstring(address), strLength + 1);

    return strLength;
}

std::string get_string_by_address(uint32_t address)
{
    uint32_t strLength = get_dword(address + 4);
    return w2s(std::wstring(get_wstring(address), strLength));
}

std::string get_string_by_str_addr(uint32_t addr)
{
    uint32_t strLength = get_dword(addr + 4);
    return strLength ? std::string(get_string(addr), strLength) : std::string();
}

std::string get_string_by_wstr_addr(uint32_t addr)
{
    uint32_t strLength = get_dword(addr + 4);
    return strLength ? w2s(std::wstring(get_wstring(addr), strLength)) : std::string();
}

uint32_t get_memory_int_by_address(void *hProcess, uint32_t address)
{
    uint32_t value = 0;

    unsigned char data[4] = { 0 };
    if (ReadProcessMemory(hProcess, (LPVOID)address, data, 4, 0)) {
        value = data[0] & 0xFF;
        value |= ((data[1] << 8) & 0xFF00);
        value |= ((data[2] << 16) & 0xFF0000);
        value |= ((data[3] << 24) & 0xFF000000);
    }

    return value;
}

std::wstring get_unicode_info_by_address(void *hProcess, uint32_t address)
{
    std::wstring value = L"";

    uint32_t strAddress = get_memory_int_by_address(hProcess, address);
    uint32_t strLen     = get_memory_int_by_address(hProcess, address + 0x4);
    if (strLen > 500)
        return value;

    wchar_t cValue[500] = { 0 };
    memset(cValue, 0, sizeof(cValue) / sizeof(wchar_t));
    if (ReadProcessMemory(hProcess, (LPVOID)strAddress, cValue, (strLen + 1) * 2, 0)) {
        value = std::wstring(cValue);
    }

    return value;
}

void dbg_msg(const char *format, ...)
{
    // initialize use of the variable argument array
    va_list vaArgs;
    va_start(vaArgs, format);

    // reliably acquire the size
    // from a copy of the variable argument array
    // and a functionally reliable call to mock the formatting
    va_list vaArgsCopy;
    va_copy(vaArgsCopy, vaArgs);
    const int iLen = std::vsnprintf(NULL, 0, format, vaArgsCopy);
    va_end(vaArgsCopy);

    // return a formatted string without risking memory mismanagement
    // and without assuming any compiler or platform specific behavior
    std::vector<char> zc(iLen + 1);
    std::vsnprintf(zc.data(), zc.size(), format, vaArgs);
    va_end(vaArgs);
    std::string strText(zc.data(), iLen);

    OutputDebugStringA(strText.c_str());
}

} // namespace util
