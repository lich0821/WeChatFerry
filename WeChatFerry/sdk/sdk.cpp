#include "Shlwapi.h"
#include "framework.h"
#include <filesystem>
#include <process.h>
#include <tlhelp32.h>

#include "injector.h"
#include "sdk.h"
#include "util.h"

#define WCF_LOCK ".wcf.lock"

static bool debugMode             = false;
static HANDLE wcProcess           = NULL;
static HMODULE spyBase            = NULL;
static WCHAR spyDllPath[MAX_PATH] = { 0 };

static int GetDllPath(bool debug, wchar_t *dllPath)
{
    GetModuleFileName(GetModuleHandle(WECHATSDKDLL), spyDllPath, MAX_PATH);
    PathRemoveFileSpec(spyDllPath);
    if (debug) {
        PathAppend(spyDllPath, WECHATINJECTDLL_DEBUG);
    } else {
        PathAppend(spyDllPath, WECHATINJECTDLL);
    }

    if (!PathFileExists(spyDllPath)) {
        MessageBox(NULL, spyDllPath, L"文件不存在", 0);
        return ERROR_FILE_NOT_FOUND;
    }

    return 0;
}

int WxInitSDK(bool debug, int port)
{
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

    if (!CallDllFuncEx(wcProcess, spyDllPath, spyBase, "InitSpy", (LPVOID)&pp, sizeof(PortPath_t), NULL)) {
        MessageBox(NULL, L"初始化失败", L"WxInitSDK", 0);
        return -1;
    }

#ifdef WCF
    FILE *fd = fopen(WCF_LOCK, "wb");
    if (fd == NULL) {
        MessageBox(NULL, L"无法打开lock文件", L"WxInitSDK", 0);
        return -2;
    }
    fwrite((uint8_t *)&debug, sizeof(debug), 1, fd);
    fwrite((uint8_t *)&spyBase, sizeof(spyBase), 1, fd);
    fclose(fd);
#endif
    debugMode = debug;
    return 0;
}

int WxDestroySDK()
{
    int status = 0;
#ifdef WCF
    bool debug;
    DWORD pid = GetWeChatPid();
    if (pid == 0) {
        MessageBox(NULL, L"微信未运行", L"WxDestroySDK", 0);
        return status;
    }

    wcProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (wcProcess == NULL) {
        MessageBox(NULL, L"微信未运行", L"WxDestroySDK", 0);
        return -1;
    }

    FILE *fd = fopen(WCF_LOCK, "rb");
    if (fd == NULL) {
        MessageBox(NULL, L"无法打开lock文件", L"WxDestroySDK", 0);
        return -2;
    }
    fread((uint8_t *)&debug, sizeof(debug), 1, fd);
    fread((uint8_t *)&spyBase, sizeof(spyBase), 1, fd);
    fclose(fd);
    status = GetDllPath(debug, spyDllPath);
#else
    status = GetDllPath(debugMode, spyDllPath);
#endif

    if (status != 0) {
        return status;
    }

    if (!CallDllFunc(wcProcess, spyDllPath, spyBase, "CleanupSpy", NULL, NULL)) {
        return -1;
    }

    if (!EjectDll(wcProcess, spyBase)) {
        return -1; // TODO: Unify error codes
    }

    return 0;
}
