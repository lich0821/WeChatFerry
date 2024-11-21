#include "Shlwapi.h"
#include "framework.h"
#include <filesystem>
#include <process.h>
#include <tlhelp32.h>

#include "injector.h"
#include "sdk.h"
#include "util.h"
#include <string>
#include <vector>

using namespace std;

struct WxProcessInfo
{
    DWORD pid;
    HANDLE wcProcess;
    HMODULE spyBase;
    bool injected;
    WCHAR spyDllPath[MAX_PATH];
    WxProcessInfo(DWORD pid)
    {
        this->pid = pid;
        this->wcProcess = NULL;
        this->spyBase = NULL;
        this->injected = false;
        this->spyDllPath[0] = 0;
    }
};

static vector<WxProcessInfo> wxProcess;

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

int WxInitSDK(bool debug, int port, int index)
{
    int status = 0;

    if (index < 0) {
        clearAllSDK();
        DWORD wcPid = 0;
        status = OpenWeChat(&wcPid);
        if (status != 0) {
            MessageBox(NULL, L"打开微信失败", L"WxInitSDK", 0);
            return status;
        }
        Sleep(2000); // 等待微信打开
        wxProcess.push_back(WxProcessInfo(wcPid));
        index = 0;
    }

    if (index + 1 > wxProcess.size()) {
        MessageBox(NULL, L"参数错误: index >= wxProcess.size", L"WxInitSDK", 0);
        return -1;
    }

    status = GetDllPath(debug, wxProcess[index].spyDllPath);
    if (status != 0)
    {
        return status;
    }

    if (!IsProcessX64(wxProcess[index].pid)) {
        MessageBox(NULL, L"只支持 64 位微信", L"WxInitSDK", 0);
        return -1;
    }

    wxProcess[index].wcProcess = InjectDll(wxProcess[index].pid, wxProcess[index].spyDllPath, &wxProcess[index].spyBase);
    if (wxProcess[index].wcProcess == NULL) {
        MessageBox(NULL, L"注入失败", L"WxInitSDK", 0);
        return -1;
    }

    PortPath_t pp = {0};
    pp.port = port;
    sprintf_s(pp.path, MAX_PATH, "%s", std::filesystem::current_path().string().c_str());

    if (!CallDllFuncEx(wxProcess[index].wcProcess, wxProcess[index].spyDllPath, wxProcess[index].spyBase, "InitSpy", (LPVOID)&pp, sizeof(PortPath_t), NULL)) {
        MessageBox(NULL, L"初始化失败", L"WxInitSDK", 0);
        return -1;
    }

    wxProcess[index].injected = true;
    return 0;
}

int WxDestroySDK(int index)
{
    if (index + 1 > wxProcess.size()) {
        MessageBox(NULL, L"参数错误: index >= wxProcess.size", L"WxInitSDK", 0);
        return -1;
    }

    if (!wxProcess[index].injected) {
        return -1;
    }

    if (!CallDllFunc(wxProcess[index].wcProcess, wxProcess[index].spyDllPath, wxProcess[index].spyBase, "CleanupSpy", NULL)) {
        return -2;
    }

    if (!EjectDll(wxProcess[index].wcProcess, wxProcess[index].spyBase)) {
        return -3; // TODO: Unify error codes
    }

    wxProcess[index].injected = false;
    wxProcess[index].spyBase = NULL;
    wxProcess[index].wcProcess = NULL;

    return 0;
}

int EnumWeChatProcess()
{
    clearAllSDK();
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32 = {sizeof(PROCESSENTRY32)};
    while (Process32Next(hSnapshot, &pe32)) {
        std::wstring strProcess = pe32.szExeFile;
        if (strProcess == WECHAREXE) {
            wxProcess.push_back(WxProcessInfo(pe32.th32ProcessID));
        }
    }
    CloseHandle(hSnapshot);
    return (int)wxProcess.size();
}

void clearAllSDK()
{
    for (int i = 0; i < wxProcess.size(); i++) {
        WxDestroySDK(i);
    }
    wxProcess.clear();
}
