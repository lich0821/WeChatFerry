#include "Shlwapi.h"
#include "framework.h"
#include <process.h>
#include <tlhelp32.h>

#include "injector.h"
#include "log.h"
#include "sdk.h"
#include "util.h"

#define WCF_LOCK ".wcf.lock"

static bool debugMode             = false;
static HANDLE wcProcess           = NULL;
static HMODULE spyBase            = NULL;
static WCHAR spyDllPath[MAX_PATH] = { 0 };

static int GetDllPath(bool debug, wchar_t *dllPath)
{
    InitLogger();
    GetModuleFileName(GetModuleHandle(WECHATSDKDLL), spyDllPath, MAX_PATH);
    PathRemoveFileSpec(spyDllPath);
    if (debug) {
        PathAppend(spyDllPath, WECHATINJECTDLL_DEBUG);
    } else {
        PathAppend(spyDllPath, WECHATINJECTDLL);
    }

    if (!PathFileExists(spyDllPath)) {
        LOG_ERROR("DLL does not exists: {}.", Wstring2String(spyDllPath));
        return ERROR_FILE_NOT_FOUND;
    }

    return 0;
}

int WxInitSDK(bool debug)
{
    int status  = 0;
    DWORD wcPid = 0;

    status = GetDllPath(debug, spyDllPath);
    if (status != 0) {
        return status;
    }

    status = OpenWeChat(&wcPid);
    if (status != 0) {
        LOG_ERROR("OpenWeChat failed: {}.", status);
        return status;
    }

    Sleep(2000); // 等待微信打开
    wcProcess = InjectDll(wcPid, spyDllPath, &spyBase);
    if (wcProcess == NULL) {
        LOG_ERROR("Failed to Inject DLL into WeChat.");
        return -1;
    }

    if (!CallDllFunc(wcProcess, spyDllPath, spyBase, "InitSpy", NULL)) {
        LOG_ERROR("Failed to InitSpy.");
        return -1;
    }

#ifdef WCF
    FILE *fd = fopen(WCF_LOCK, "wb");
    if (fd == NULL) {
        LOG_ERROR("Failed to open {}.", WCF_LOCK);
        return -2;
    }
    fwrite((uint8_t *)&debug, sizeof(debug), 1, fd);
    fwrite((uint8_t *)&spyBase, sizeof(spyBase), 1, fd);
    fclose(fd);
#endif
    debugMode = debug;
    LOG_INFO("WxInitSDK done.");
    return 0;
}

int WxDestroySDK()
{
    int status = 0;
#ifdef WCF
    bool debug;
    DWORD pid = GetWeChatPid();
    if (pid == 0) {
        LOG_ERROR("WeChat is not running.");
        return status;
    }

    wcProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (wcProcess == NULL) {
        LOG_ERROR("WeChat is not running.");
        return -1;
    }

    FILE *fd = fopen(WCF_LOCK, "rb");
    if (fd == NULL) {
        LOG_ERROR("Failed to open {}.", WCF_LOCK);
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

    if (!CallDllFunc(wcProcess, spyDllPath, spyBase, "CleanupSpy", NULL)) {
        LOG_ERROR("Failed to CleanupSpy.");
        return -1;
    }

    if (!EjectDll(wcProcess, spyBase)) {
        LOG_ERROR("Failed to Eject DLL.");
        return -1; // TODO: Unify error codes
    }
    LOG_INFO("WxDestroySDK done.");
    return 0;
}
