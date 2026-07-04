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

// 检查目标进程是否已加载 spy 模块，用于 start 幂等：重复 start 会二次注入，
// 导致 RPC 端口重复绑定。比 lock 文件可靠——微信重启换 pid 后 lock 会残留成
// 过期状态，而模块列表反映的是当前真实加载情况。
static bool IsSpyInjected(uint32_t pid)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    bool found        = false;
    MODULEENTRY32W me = { 0 };
    me.dwSize         = sizeof(me);
    if (Module32FirstW(snapshot, &me)) {
        do {
            if (_wcsicmp(me.szModule, WECHATINJECTDLL) == 0 || _wcsicmp(me.szModule, WECHATINJECTDLL_DEBUG) == 0) {
                found = true;
                break;
            }
        } while (Module32NextW(snapshot, &me));
    }
    CloseHandle(snapshot);
    return found;
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
        return WX_ERR_DLL_NOT_FOUND;
    }

    return WX_OK;
}

int WxInitSDK(bool debug, int port)
{
    int status  = 0;
    uint32_t wcPid = 0;

    status = GetDllPath(debug, spyDllPath);
    if (status != 0) {
        return status;
    }

    if (util::open_wechat(&wcPid) != ERROR_SUCCESS) {
        ReportError(L"打开微信失败", L"WxInitSDK");
        return WX_ERR_OPEN_WECHAT;
    }

    if (IsSpyInjected(wcPid)) {
        ReportError(L"spy 已注入，请勿重复 start（如需重启请先 stop）", L"WxInitSDK");
        return WX_ERR_ALREADY_INJECTED;
    }

    Sleep(2000); // 等待微信打开
    wcProcess = InjectDll(wcPid, spyDllPath, &spyBase);
    if (wcProcess == NULL) {
        ReportError(L"注入失败", L"WxInitSDK");
        return WX_ERR_INJECT;
    }

    WCHAR moduleDir[MAX_PATH] = { 0 };
    GetModuleDir(moduleDir);

    PortPath_t pp = { 0 };
    pp.port       = port;
    sprintf_s(pp.path, MAX_PATH, "%s", util::w2s(moduleDir).c_str());

    if (!CallDllFuncEx(wcProcess, spyDllPath, spyBase, "InitSpy", (LPVOID)&pp, sizeof(PortPath_t), NULL)) {
        ReportError(L"初始化失败", L"WxInitSDK");
        return WX_ERR_INIT_SPY;
    }

#ifdef WCF
    WCHAR lockPath[MAX_PATH] = { 0 };
    GetLockPath(lockPath);
    FILE *fd = _wfopen(lockPath, L"wb");
    if (fd == NULL) {
        ReportError(L"无法打开lock文件", L"WxInitSDK");
        return WX_ERR_LOCK;
    }
    fwrite((uint8_t *)&debug, sizeof(debug), 1, fd);
    fwrite((uint8_t *)&spyBase, sizeof(spyBase), 1, fd);
    fclose(fd);
#endif
    debugMode = debug;
    return WX_OK;
}

int WxDestroySDK()
{
    int status = WX_OK;
#ifdef WCF
    bool debug;
    uint32_t pid = util::get_wechat_pid();
    if (pid == 0) {
        ReportError(L"微信未运行", L"WxDestroySDK");
        return WX_ERR_WECHAT_NOT_RUNNING;
    }

    wcProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (wcProcess == NULL) {
        ReportError(L"打开微信进程失败", L"WxDestroySDK");
        return WX_ERR_WECHAT_NOT_RUNNING;
    }

    WCHAR lockPath[MAX_PATH] = { 0 };
    GetLockPath(lockPath);
    FILE *fd = _wfopen(lockPath, L"rb");
    if (fd == NULL) {
        ReportError(L"无法打开lock文件", L"WxDestroySDK");
        return WX_ERR_LOCK;
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
        ReportError(L"清理 spy 失败", L"WxDestroySDK");
        return WX_ERR_EJECT;
    }

    if (!EjectDll(wcProcess, spyBase)) {
        return WX_ERR_EJECT;
    }

    return WX_OK;
}
