#include "Shlwapi.h"
#include "framework.h"
#include <filesystem>
#include <process.h>
#include <tlhelp32.h>

#include "injector.h"
#include "sdk.h"
#include "util.h"

#define WCF_LOCK L".wcf.lock"

static bool debugMode             = false;
static HANDLE wcProcess           = NULL;
static HMODULE spyBase            = NULL;
static WCHAR spyDllPath[MAX_PATH] = { 0 };

void WxSetGuiMode(bool enable) { g_guiMode = enable; }

// 取 sdk.dll 所在目录（注入器自身目录），用作 lock 文件与 spy 工作目录的锚点，
// 避免依赖调用方 CWD——否则 start/stop 若 CWD 不同，stop 找不到 lock。
static void GetModuleDir(WCHAR *dir)
{
    GetModuleFileName(GetModuleHandle(WECHATSDKDLL), dir, MAX_PATH);
    PathRemoveFileSpec(dir);
}

static void GetLockPath(WCHAR *lockPath)
{
    GetModuleDir(lockPath);
    PathAppend(lockPath, WCF_LOCK);
}

static int GetDllPath(bool debug, wchar_t *dllPath)
{
    GetModuleDir(spyDllPath);
    if (debug) {
        PathAppend(spyDllPath, WECHATINJECTDLL_DEBUG);
    } else {
        PathAppend(spyDllPath, WECHATINJECTDLL);
    }

    if (!PathFileExists(spyDllPath)) {
        ReportError(spyDllPath, L"文件不存在");
        return ERROR_FILE_NOT_FOUND;
    }

    return 0;
}

int WxInitSDK(bool debug, int port)
{
    int status  = 0;
    uint32_t wcPid = 0;

    status = GetDllPath(debug, spyDllPath);
    if (status != 0) {
        return status;
    }

    status = util::open_wechat(&wcPid);
    if (status != 0) {
        ReportError(L"打开微信失败", L"WxInitSDK");
        return status;
    }

    Sleep(2000); // 等待微信打开
    wcProcess = InjectDll(wcPid, spyDllPath, &spyBase);
    if (wcProcess == NULL) {
        ReportError(L"注入失败", L"WxInitSDK");
        return -1;
    }

    WCHAR moduleDir[MAX_PATH] = { 0 };
    GetModuleDir(moduleDir);

    PortPath_t pp = { 0 };
    pp.port       = port;
    sprintf_s(pp.path, MAX_PATH, "%s", util::w2s(moduleDir).c_str());

    if (!CallDllFuncEx(wcProcess, spyDllPath, spyBase, "InitSpy", (LPVOID)&pp, sizeof(PortPath_t), NULL)) {
        ReportError(L"初始化失败", L"WxInitSDK");
        return -1;
    }

#ifdef WCF
    WCHAR lockPath[MAX_PATH] = { 0 };
    GetLockPath(lockPath);
    FILE *fd = _wfopen(lockPath, L"wb");
    if (fd == NULL) {
        ReportError(L"无法打开lock文件", L"WxInitSDK");
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
    uint32_t pid = util::get_wechat_pid();
    if (pid == 0) {
        ReportError(L"微信未运行", L"WxDestroySDK");
        return status;
    }

    wcProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (wcProcess == NULL) {
        ReportError(L"微信未运行", L"WxDestroySDK");
        return -1;
    }

    WCHAR lockPath[MAX_PATH] = { 0 };
    GetLockPath(lockPath);
    FILE *fd = _wfopen(lockPath, L"rb");
    if (fd == NULL) {
        ReportError(L"无法打开lock文件", L"WxDestroySDK");
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
