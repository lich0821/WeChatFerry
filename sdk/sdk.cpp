#include "Shlwapi.h"
#include "framework.h"
#include <process.h>
#include <tlhelp32.h>

#include "injector.h"
#include "log.h"
#include "sdk.h"
#include "util.h"

static DWORD wcPid                = 0;
static HANDLE wcProcess           = NULL;
static HMODULE spyBase            = NULL;
static WCHAR spyDllPath[MAX_PATH] = { 0 };

int WxInitSDK()
{
    int status = 0;
    InitLogger();
    LOG_INFO("WxInitSDK.");
    GetModuleFileName(GetModuleHandle(WECHATSDKDLL), spyDllPath, MAX_PATH);
    PathRemoveFileSpec(spyDllPath);
    PathAppend(spyDllPath, WECHATINJECTDLL);

    if (!PathFileExists(spyDllPath)) {
        LOG_ERROR("DLL does not exists.");
        return ERROR_FILE_NOT_FOUND;
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
#if 0
    do {
        if (!CallDllFunc(wcProcess, spyDllPath, spyBase, "IsLogin", (DWORD *)&status)) {
            LOG_ERROR("Failed to check login status.");
            return -1;
        }
        Sleep(1000);
    } while (status == 0);
#endif
    return 0;
}

int WxDestroySDK()
{
    LOG_INFO("WxDestroySDK");
    if (!CallDllFunc(wcProcess, spyDllPath, spyBase, "CleanupSpy", NULL)) {
        LOG_ERROR("Failed to CleanupSpy.");
        return -1;
    }

    if (!EjectDll(wcProcess, spyBase)) {
        LOG_ERROR("Failed to Eject DLL.");
        return -1; // TODO: Unify error codes
    }

    return 0;
}
