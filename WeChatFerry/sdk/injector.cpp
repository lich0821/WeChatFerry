#include "injector.h"

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
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, pRemoteAddress, 0, NULL);
    if (hThread == NULL) {
        MessageBox(NULL, L"LoadLibrary 调用失败", L"InjectDll", 0);
        return NULL;
    }

    WaitForSingleObject(hThread, -1);
    GetExitCodeThread(hThread, (LPDWORD)injectedBase);
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemoteAddress, 0, MEM_RELEASE);
    // CloseHandle(hProcess); // Close when exit

    return hProcess;
}

bool EjectDll(HANDLE process, HMODULE dllBase)
{
    HANDLE hThread = NULL;

    // 使目标进程调用 FreeLibrary，卸载 DLL
    hThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, (LPVOID)dllBase, 0, NULL);
    if (hThread == NULL) {
        MessageBox(NULL, L"FreeLibrary 调用失败!", L"EjectDll", 0);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(process);
    return true;
}

static void *GetFuncAddr(LPCWSTR dllPath, HMODULE dllBase, LPCSTR funcName)
{
    HMODULE hLoaded = LoadLibrary(dllPath);
    if (hLoaded == NULL) {
        return NULL;
    }

    void *absAddr = GetProcAddress(hLoaded, funcName);
    DWORD offset  = (DWORD)absAddr - (DWORD)hLoaded;

    FreeLibrary(hLoaded);

    return (void *)((DWORD)dllBase + offset);
}

bool CallDllFunc(HANDLE process, LPCWSTR dllPath, HMODULE dllBase, LPCSTR funcName, LPVOID parameter, DWORD *ret)
{
    void *pFunc = GetFuncAddr(dllPath, dllBase, funcName);
    if (pFunc == NULL) {
        return false;
    }

    HANDLE hThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)pFunc, parameter, 0, NULL);
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
                   DWORD *ret)
{
    void *pFunc = GetFuncAddr(dllPath, dllBase, funcName);
    if (pFunc == NULL) {
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
