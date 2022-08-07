#include "Shlwapi.h"
#include "framework.h"
#include <string.h>
#include <strsafe.h>
#include <wchar.h>

#include "util.h"

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "Version.lib")

using namespace std;

int GetWeChatPath(wchar_t *path);
int GetWeChatWinDLLPath(wchar_t *path);
int GetWeChatVersion(wchar_t *version);
bool GetFileVersion(const wchar_t *filePath, wchar_t *version);

int GetWeChatPath(wchar_t *path)
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

int GetWeChatWinDLLPath(wchar_t *path)
{
    int ret = GetWeChatPath(path);
    if (ret != ERROR_SUCCESS) {
        return ret;
    }

    PathRemoveFileSpecW(path);
    PathAppendW(path, WECHATWINDLL);
    if (!PathFileExists(path)) {
        // 微信从（大约）3.7开始，增加了一层版本目录: [3.7.0.29]
        PathRemoveFileSpec(path);
        _wfinddata_t findData;
        wstring dir     = wstring(path) + L"\\[*.*";
        intptr_t handle = _wfindfirst(dir.c_str(), &findData);
        if (handle == -1) { // 检查是否成功
            return -1;
        }
        wstring dllPath = wstring(path) + L"\\" + findData.name;
        wcscpy_s(path, MAX_PATH, dllPath.c_str());
        PathAppend(path, WECHATWINDLL);
    }

    return ret;
}

int GetWeChatVersion(wchar_t *version)
{
    WCHAR Path[MAX_PATH] = { 0 };

    int ret = GetWeChatWinDLLPath(Path);
    if (ret != ERROR_SUCCESS) {
        return ret;
    }

    ret = GetFileVersion(Path, version);

    return ret;
}

bool GetFileVersion(const wchar_t *filePath, wchar_t *version)
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

int OpenWeChat(DWORD *pid)
{
    int ret                = -1;
    STARTUPINFO si         = { sizeof(si) };
    WCHAR Path[MAX_PATH]   = { 0 };
    PROCESS_INFORMATION pi = { 0 };

    ret = GetWeChatPath(Path);
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

int GetWstringByAddress(DWORD address, wchar_t *buffer, DWORD buffer_size)
{
    DWORD strLength = GET_DWORD(address + 4);
    if (strLength == 0) {
        return 0;
    } else if (strLength > buffer_size) {
        strLength = buffer_size - 1;
    }

    wmemcpy_s(buffer, strLength + 1, GET_WSTRING(address), strLength + 1);

    return strLength;
}

BSTR GetBstrByAddress(DWORD address)
{
    wchar_t *p = GET_WSTRING(address);
    if (p == NULL) {
        return NULL;
    }

    return SysAllocStringLen(GET_WSTRING(address), GET_DWORD(address + 4));
}

wstring GetWstringFromBstr(BSTR p)
{
    wstring ws = L"";
    if (p != NULL) {
        ws = wstring(p);
        SysFreeString(p);
    }
    return ws;
}

BSTR GetBstrFromString(const char *str)
{
    int wslen = MultiByteToWideChar(CP_ACP, 0, str, strlen(str), 0, 0);
    BSTR bstr = SysAllocStringLen(0, wslen);
    MultiByteToWideChar(CP_ACP, 0, str, strlen(str), bstr, wslen);

    return bstr;
}

BSTR GetBstrFromWstring(wstring ws)
{
    if (!ws.empty()) {
        return SysAllocStringLen(ws.data(), ws.size());
    }
    return NULL;
}

void GetRpcMessage(WxMessage_t *wxMsg, RpcMessage_t rpcMsg)
{
    wxMsg->self    = rpcMsg.self;
    wxMsg->type    = rpcMsg.type;
    wxMsg->source  = rpcMsg.source;
    wxMsg->id      = GetWstringFromBstr(rpcMsg.id);
    wxMsg->xml     = GetWstringFromBstr(rpcMsg.xml);
    wxMsg->wxId    = GetWstringFromBstr(rpcMsg.wxId);
    wxMsg->roomId  = GetWstringFromBstr(rpcMsg.roomId);
    wxMsg->content = GetWstringFromBstr(rpcMsg.content);
}

DWORD GetMemoryIntByAddress(HANDLE hProcess, DWORD address)
{
    DWORD value = 0;

    unsigned char data[4] = { 0 };
    if (ReadProcessMemory(hProcess, (LPVOID)address, data, 4, 0)) {
        value = data[0] & 0xFF;
        value |= ((data[1] << 8) & 0xFF00);
        value |= ((data[2] << 16) & 0xFF0000);
        value |= ((data[3] << 24) & 0xFF000000);
    }

    return value;
}

wstring GetUnicodeInfoByAddress(HANDLE hProcess, DWORD address)
{
    wstring value = L"";

    DWORD strAddress = GetMemoryIntByAddress(hProcess, address);
    DWORD strLen     = GetMemoryIntByAddress(hProcess, address + 0x4);
    if (strLen > 500)
        return value;

    wchar_t cValue[500] = { 0 };
    memset(cValue, 0, sizeof(cValue) / sizeof(wchar_t));
    if (ReadProcessMemory(hProcess, (LPVOID)strAddress, cValue, (strLen + 1) * 2, 0)) {
        value = wstring(cValue);
    }

    return value;
}
