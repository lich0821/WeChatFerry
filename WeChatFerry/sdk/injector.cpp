#include "injector.h"

#include <filesystem>

#include "psapi.h"

#include "util.h"

using namespace std;

static void handle_injection_error(HANDLE process, LPVOID remote_address, const std::string &error_msg)
{
    util::MsgBox(NULL, error_msg.c_str(), "Error", MB_ICONERROR);
    if (remote_address) {
        VirtualFreeEx(process, remote_address, 0, MEM_RELEASE);
    }
    if (process) {
        CloseHandle(process);
    }
}

HMODULE get_target_module_base(HANDLE process, const string &dll)
{
    DWORD needed;
    HMODULE modules[512];
    if (!EnumProcessModulesEx(process, modules, sizeof(modules), &needed, LIST_MODULES_64BIT)) {
        util::MsgBox(NULL, "获取模块失败", "get_target_module_base", 0);
        return NULL;
    }

    DWORD count = needed / sizeof(HMODULE);
    char module_name[MAX_PATH];
    for (DWORD i = 0; i < count; i++) {
        GetModuleBaseNameA(process, modules[i], module_name, sizeof(module_name));
        if (!strncmp(dll.c_str(), module_name, dll.size())) {
            return modules[i];
        }
    }
    return NULL;
}

HANDLE inject_dll(DWORD pid, const string &dll_path, HMODULE *injected_base)
{
    SIZE_T path_size = dll_path.size() + 1;

    // 1. 打开目标进程
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        util::MsgBox(NULL, "打开进程失败", "inject_dll", 0);
        return NULL;
    }

    // 2. 在目标进程的内存里开辟空间
    LPVOID pRemoteAddress = VirtualAllocEx(hProcess, NULL, path_size, MEM_COMMIT, PAGE_READWRITE);
    if (!pRemoteAddress) {
        handle_injection_error(hProcess, NULL, "DLL 路径写入失败");
        return NULL;
    }

    // 3. 把 dll 的路径写入到目标进程的内存空间中
    WriteProcessMemory(hProcess, pRemoteAddress, dll_path.c_str(), path_size, NULL);

    // 4. 创建一个远程线程，让目标进程调用 LoadLibrary
    HMODULE k32 = GetModuleHandleA("kernel32.dll");
    if (!k32) {
        handle_injection_error(hProcess, pRemoteAddress, "获取 kernel32 失败");
        return NULL;
    }

    FARPROC libAddr = GetProcAddress(k32, "LoadLibraryA");
    if (!libAddr) {
        handle_injection_error(hProcess, pRemoteAddress, "获取 LoadLibrary 失败");
        return NULL;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)libAddr, pRemoteAddress, 0, NULL);
    if (!hThread) {
        handle_injection_error(hProcess, pRemoteAddress, "CreateRemoteThread 失败");
        return NULL;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    *injected_base = get_target_module_base(hProcess, filesystem::path(dll_path).filename().string());

    VirtualFreeEx(hProcess, pRemoteAddress, 0, MEM_RELEASE);
    return hProcess;
}

bool eject_dll(HANDLE process, HMODULE dll_base)
{
    HMODULE k32 = GetModuleHandleA("kernel32.dll");
    if (!k32) {
        util::MsgBox(NULL, "获取 kernel32 失败", "eject_dll", 0);
        return false;
    }

    FARPROC libAddr = GetProcAddress(k32, "FreeLibraryAndExitThread");
    if (!libAddr) {
        util::MsgBox(NULL, "获取 FreeLibrary 失败", "eject_dll", 0);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)libAddr, (LPVOID)dll_base, 0, NULL);
    if (!hThread) {
        util::MsgBox(NULL, "FreeLibrary 调用失败!", "eject_dll", 0);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(process);
    return true;
}

static uint64_t get_func_offset(const string &dll_path, const string &func_name)
{
    HMODULE dll = LoadLibraryA(dll_path.c_str());
    if (!dll) {
        util::MsgBox(NULL, "获取 DLL 失败", "get_func_offset", 0);
        return 0;
    }

    LPVOID absAddr  = GetProcAddress(dll, func_name.c_str());
    uint64_t offset = reinterpret_cast<uint64_t>(absAddr) - reinterpret_cast<uint64_t>(dll);
    FreeLibrary(dll);

    return offset;
}

bool call_dll_func(HANDLE process, const string &dll_path, HMODULE dll_base, const string &func_name, DWORD *ret)
{
    uint64_t offset = get_func_offset(dll_path, func_name);
    if (offset == 0 || offset > (UINT64_MAX - reinterpret_cast<uint64_t>(dll_base))) {
        return false; // 避免溢出
    }
    uint64_t pFunc = reinterpret_cast<uint64_t>(dll_base) + offset;
    HANDLE hThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)pFunc, NULL, 0, NULL);
    if (!hThread) {
        return false;
    }
    WaitForSingleObject(hThread, INFINITE);
    if (ret) {
        GetExitCodeThread(hThread, ret);
    }

    CloseHandle(hThread);
    return true;
}

bool call_dll_func_ex(HANDLE process, const string &dll_path, HMODULE dll_base, const string &func_name,
                      LPVOID parameter, size_t size, DWORD *ret)
{
    uint64_t offset = get_func_offset(dll_path, func_name);
    if (offset == 0 || offset > (UINT64_MAX - reinterpret_cast<uint64_t>(dll_base))) {
        return false; // 避免溢出
    }
    uint64_t pFunc        = reinterpret_cast<uint64_t>(dll_base) + offset;
    LPVOID pRemoteAddress = VirtualAllocEx(process, NULL, size, MEM_COMMIT, PAGE_READWRITE);
    if (!pRemoteAddress) {
        util::MsgBox(NULL, "申请内存失败", "call_dll_func_ex", 0);
        return false;
    }

    WriteProcessMemory(process, pRemoteAddress, parameter, size, NULL);

    HANDLE hThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)pFunc, pRemoteAddress, 0, NULL);
    if (!hThread) {
        VirtualFreeEx(process, pRemoteAddress, 0, MEM_RELEASE);
        util::MsgBox(NULL, "远程调用失败", "call_dll_func_ex", 0);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);
    VirtualFreeEx(process, pRemoteAddress, 0, MEM_RELEASE);
    if (ret) {
        GetExitCodeThread(hThread, ret);
    }

    CloseHandle(hThread);
    return true;
}
