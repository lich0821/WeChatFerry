#include "framework.h"
#include "psapi.h"
#include <filesystem>
#include <string>

#include "injector.h"
#include "util.h"

using namespace std;

HMODULE GetTargetModuleBase(HANDLE process, string dll)
{
    DWORD cbNeeded;
    HMODULE moduleHandleList[512];
    BOOL ret = EnumProcessModulesEx(process, moduleHandleList, sizeof(moduleHandleList), &cbNeeded, LIST_MODULES_64BIT);
    if (!ret) {
        MessageBox(NULL, L"获取模块失败", L"GetTargetModuleBase", 0);
        return NULL;
    }

    if (cbNeeded > sizeof(moduleHandleList)) {
        MessageBox(NULL, L"模块数量过多", L"GetTargetModuleBase", 0);
        return NULL;
    }
    DWORD processCount = cbNeeded / sizeof(HMODULE);

    char moduleName[32];
    for (DWORD i = 0; i < processCount; i++) {
        GetModuleBaseNameA(process, moduleHandleList[i], moduleName, 32);
        if (!strncmp(dll.c_str(), moduleName, dll.size())) {
            return moduleHandleList[i];
        }
    }
    return NULL;
}

HANDLE InjectDll(DWORD pid, LPCWSTR dllPath, HMODULE *injectedBase)
{
    HANDLE hThread;
    SIZE_T cszDLL = (wcslen(dllPath) + 1) * sizeof(WCHAR);

    // 1. 打开目标进程
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) {
        MessageBox(NULL, L"打开进程失败", L"InjectDll", 0);
        return NULL;
    }

    // 2. 在目标进程的内存里开辟空间
    LPVOID pRemoteAddress = VirtualAllocEx(hProcess, NULL, cszDLL, MEM_COMMIT, PAGE_READWRITE);
    if (pRemoteAddress == NULL) {
        MessageBox(NULL, L"DLL 路径写入失败", L"InjectDll", 0);
        return NULL;
    }

    // 3. 把 dll 的路径写入到目标进程的内存空间中
    WriteProcessMemory(hProcess, pRemoteAddress, dllPath, cszDLL, NULL);

    // 3. 创建一个远程线程，让目标进程调用 LoadLibrary
    HMODULE k32 = GetModuleHandle(L"kernel32.dll");
    if (k32 == NULL) {
        MessageBox(NULL, L"获取 kernel32 失败", L"InjectDll", 0);
        return NULL;
    }

    FARPROC libAddr = GetProcAddress(k32, "LoadLibraryW");
    if (!libAddr) {
        MessageBox(NULL, L"获取 LoadLibrary 失败", L"InjectDll", 0);
        return NULL;
    }

    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)libAddr, pRemoteAddress, 0, NULL);
    if (hThread == NULL) {
        VirtualFreeEx(hProcess, pRemoteAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        MessageBox(NULL, L"CreateRemoteThread 失败", L"InjectDll", 0);
        return NULL;
    }

    WaitForSingleObject(hThread, -1);
    CloseHandle(hThread);

    *injectedBase = GetTargetModuleBase(hProcess, filesystem::path(Wstring2String(dllPath)).filename().string());

    VirtualFreeEx(hProcess, pRemoteAddress, 0, MEM_RELEASE);
    // CloseHandle(hProcess); // Close when exit

    return hProcess;
}

bool EjectDll(HANDLE process, HMODULE dllBase)
{
    HANDLE hThread = NULL;

    // 使目标进程调用 FreeLibrary，卸载 DLL
    HMODULE k32 = GetModuleHandle(L"kernel32.dll");
    if (k32 == NULL) {
        MessageBox(NULL, L"获取 kernel32 失败", L"InjectDll", 0);
        return NULL;
    }

    FARPROC libAddr = GetProcAddress(k32, "FreeLibraryAndExitThread");
    if (!libAddr) {
        MessageBox(NULL, L"获取 FreeLibrary 失败", L"InjectDll", 0);
        return NULL;
    }
    hThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)libAddr, (LPVOID)dllBase, 0, NULL);
    if (hThread == NULL) {
        MessageBox(NULL, L"FreeLibrary 调用失败!", L"EjectDll", 0);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(process);
    return true;
}

static UINT64 GetFuncOffset(LPCWSTR dllPath, LPCSTR funcName)
{
    HMODULE dll = LoadLibrary(dllPath);
    if (dll == NULL) {
        MessageBox(NULL, L"获取 DLL 失败", L"GetFuncOffset", 0);
        return 0;
    }

    LPVOID absAddr = GetProcAddress(dll, funcName);
    UINT64 offset  = (UINT64)absAddr - (UINT64)dll;
    FreeLibrary(dll);

    return offset;
}

bool CallDllFunc(HANDLE process, LPCWSTR dllPath, HMODULE dllBase, LPCSTR funcName, LPDWORD ret)
{
    UINT64 offset = GetFuncOffset(dllPath, funcName);
    if (offset == 0) {
        return false;
    }
    UINT64 pFunc = (UINT64)dllBase + GetFuncOffset(dllPath, funcName);
    if (pFunc <= (UINT64)dllBase) {
        return false;
    }

    HANDLE hThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)pFunc, NULL, 0, NULL);
    if (hThread == NULL) {
        return false;
    }
    WaitForSingleObject(hThread, INFINITE);
    if (ret != NULL) {
        GetExitCodeThread(hThread, ret);
    }

    CloseHandle(hThread);
    return true;
}

bool CallDllFuncEx(HANDLE process, LPCWSTR dllPath, HMODULE dllBase, LPCSTR funcName, LPVOID parameter, size_t sz,
                   LPDWORD ret)
{
    UINT64 offset = GetFuncOffset(dllPath, funcName);
    if (offset == 0) {
        return false;
    }
    UINT64 pFunc = (UINT64)dllBase + GetFuncOffset(dllPath, funcName);
    if (pFunc <= (UINT64)dllBase) {
        return false;
    }

    LPVOID pRemoteAddress = VirtualAllocEx(process, NULL, sz, MEM_COMMIT, PAGE_READWRITE);
    if (pRemoteAddress == NULL) {
        MessageBox(NULL, L"申请内存失败", L"CallDllFuncEx", 0);
        return NULL;
    }

    WriteProcessMemory(process, pRemoteAddress, parameter, sz, NULL);

    HANDLE hThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)pFunc, pRemoteAddress, 0, NULL);
    if (hThread == NULL) {
        VirtualFree(pRemoteAddress, 0, MEM_RELEASE);
        MessageBox(NULL, L"远程调用失败", L"CallDllFuncEx", 0);
        return false;
    }
    WaitForSingleObject(hThread, INFINITE);
    VirtualFree(pRemoteAddress, 0, MEM_RELEASE);
    if (ret != NULL) {
        GetExitCodeThread(hThread, ret);
    }

    CloseHandle(hThread);
    return true;
}
