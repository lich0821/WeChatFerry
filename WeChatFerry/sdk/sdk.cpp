#include "Shlwapi.h"
#include "framework.h"
#include <filesystem>
#include <process.h>
#include <tlhelp32.h>

#include "injector.h"
#include "sdk.h"
#include "util.h"

#define WCF_LOCK ".wcf.lock"

static bool dbg                   = false;
static HANDLE wcProcess           = NULL;
static HMODULE spyBase            = NULL;
static WCHAR spyDllPath[MAX_PATH] = { 0 };

static int GetDllPath(bool debug, wchar_t *dllPath)
{
    GetModuleFileName(GetModuleHandle(WCFSDKDLL), dllPath, MAX_PATH);
    PathRemoveFileSpec(dllPath);
    if (debug) {
        PathAppend(dllPath, WCFSPYDLL_DEBUG);
    } else {
        PathAppend(dllPath, WCFSPYDLL);
    }

    if (!PathFileExists(dllPath)) {
        MessageBox(NULL, dllPath, L"文件不存在", 0);
        return ERROR_FILE_NOT_FOUND;
    }

    return 0;
}

int WxInitSDK(bool debug, int port)
{
    dbg         = debug;
    int status  = 0;
    DWORD wcPid = 0;

    status = GetDllPath(debug, spyDllPath);
    if (status != 0) {
        return status;
    }

    status = OpenWeChat(&wcPid);
    if (status != 0) {
        MessageBox(NULL, L"打开微信失败", L"WxInitSDK", 0);
        return status;
    }

    Sleep(2000); // 等待微信打开
    wcProcess = InjectDll(wcPid, spyDllPath, &spyBase);
    if (wcProcess == NULL) {
        MessageBox(NULL, L"注入失败", L"WxInitSDK", 0);
        return -1;
    }

    PortPath_t pp = { 0 };
    pp.port       = port;
    sprintf_s(pp.path, MAX_PATH, "%s", std::filesystem::current_path().string().c_str());

    printf("process: %p, base: %p, path: %s\n", wcProcess, spyBase, pp.path);
    if (!CallDllFuncEx(wcProcess, spyDllPath, spyBase, "InitSpy", (LPVOID)&pp, sizeof(PortPath_t), NULL)) {
        MessageBox(NULL, L"初始化失败", L"WxInitSDK", 0);
        return -1;
    }

    return 0;
}

int WxDestroySDK()
{
    if (!CallDllFunc(wcProcess, spyDllPath, spyBase, "CleanupSpy", NULL)) {
        return -1;
    }

    if (!EjectDll(wcProcess, spyBase)) {
        return -1; // TODO: Unify error codes
    }

    return 0;
}
