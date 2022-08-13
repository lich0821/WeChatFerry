#include "injector.h"

int InjectDll(DWORD pid, const WCHAR *dllPath)
{
    HANDLE hThread;
    DWORD dwWriteSize = 0;
    // 1. 获取目标进程，并在目标进程的内存里开辟空间
    HANDLE hProcess       = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    LPVOID pRemoteAddress = VirtualAllocEx(hProcess, NULL, 1, MEM_COMMIT, PAGE_READWRITE);

    // 2. 把 dll 的路径写入到目标进程的内存空间中
    if (pRemoteAddress) {
        WriteProcessMemory(hProcess, pRemoteAddress, dllPath, wcslen(dllPath) * 2 + 2, &dwWriteSize);
    } else {
        MessageBox(NULL, L"DLL 路径写入失败", L"InjectDll", 0);
        return -1;
    }

    // 3. 创建一个远程线程，让目标进程调用 LoadLibrary
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, pRemoteAddress, NULL, NULL);
    if (hThread) {
        WaitForSingleObject(hThread, -1);
    } else {
        MessageBox(NULL, L"LoadLibrary 调用失败", L"InjectDll", 0);
        return -2;
    }
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemoteAddress, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    return 0;
}

int EjectDll(DWORD pid, const WCHAR *dllPath)
{
    DWORD dwHandle, dwID;
    HANDLE hThread    = NULL;
    DWORD dwWriteSize = 0;

    HANDLE hProcess       = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    LPVOID pRemoteAddress = VirtualAllocEx(hProcess, NULL, 1, MEM_COMMIT, PAGE_READWRITE);

    if (pRemoteAddress)
        WriteProcessMemory(hProcess, pRemoteAddress, dllPath, wcslen(dllPath) * 2 + 2, &dwWriteSize);
    else {
        MessageBox(NULL, L"DLL 路径写入失败", L"EjectDll", 0);
        return -1;
    }
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetModuleHandleW, pRemoteAddress, 0, &dwID);
    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
        GetExitCodeThread(hThread, &dwHandle);
    } else {
        MessageBox(NULL, L"GetModuleHandleW 调用失败!", L"EjectDll", 0);
        return -2;
    }
    CloseHandle(hThread);

    // 使目标进程调用 FreeLibrary，卸载 DLL
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, (LPVOID)dwHandle, 0, &dwID);
    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
    } else {
        MessageBox(NULL, L"FreeLibrary 调用失败!", L"EjectDll", 0);
        return -3;
    }
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemoteAddress, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    return 0;
}
