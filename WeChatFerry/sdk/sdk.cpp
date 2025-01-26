#include "framework.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <optional>
#include <process.h>
#include <sstream>
#include <thread>
#include <tlhelp32.h>

#include "injector.h"
#include "sdk.h"
#include "util.h"

static BOOL injected    = false;
static HANDLE wcProcess = NULL;
static HMODULE spyBase  = NULL;
static std::wstring spyDllPath;

const char *DISCLAIMER_FILE      = ".license_accepted.flag";
const char *DISCLAIMER_TEXT_FILE = "DISCLAIMER.md";

static std::optional<std::wstring> ReadDisclaimerText(const char *filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return std::nullopt; // 文件打开失败
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return String2Wstring(content);
}

static bool ShowDisclaimer()
{
    if (std::filesystem::exists(DISCLAIMER_FILE)) {
        return true;
    }

    std::optional<std::wstring> disclaimerTextOpt = ReadDisclaimerText(DISCLAIMER_TEXT_FILE);
    if (!disclaimerTextOpt.has_value() || disclaimerTextOpt->empty()) {
        MessageBox(NULL, L"免责声明文件为空或读取失败。", L"错误", MB_ICONERROR);
        return false;
    }

    std::wstring disclaimerText = *disclaimerTextOpt;

    int result = MessageBox(NULL, disclaimerText.c_str(), L"免责声明", MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2);

    if (result == IDCANCEL) {
        MessageBox(NULL, L"您拒绝了免责声明，程序将退出。", L"提示", MB_ICONINFORMATION);
        return false;
    }

    std::ofstream flagFile(DISCLAIMER_FILE, std::ios::out | std::ios::trunc);
    if (!flagFile) {
        MessageBox(NULL, L"无法创建协议标志文件。", L"错误", MB_ICONERROR);
        return false;
    }
    flagFile << "User accepted the license agreement.";

    return true;
}

static std::wstring GetDllPath(bool debug)
{
    WCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(GetModuleHandle(WCFSDKDLL), buffer, MAX_PATH);

    std::filesystem::path path(buffer);
    path.remove_filename(); // 移除文件名，保留目录路径

    path /= debug ? WCFSPYDLL_DEBUG : WCFSPYDLL;

    if (!std::filesystem::exists(path)) {
        MessageBox(NULL, path.c_str(), L"文件不存在", MB_ICONERROR);
        return L"";
    }

    return path.wstring();
}

int WxInitSDK(bool debug, int port)
{
    if (!ShowDisclaimer()) {
        exit(-1); // 用户拒绝协议，退出程序
    }

    int status  = 0;
    DWORD wcPid = 0;

    spyDllPath = GetDllPath(debug);
    if (spyDllPath.empty()) {
        return ERROR_FILE_NOT_FOUND; // DLL 文件路径不存在
    }

    status = OpenWeChat(&wcPid);
    if (status != 0) {
        MessageBox(NULL, L"打开微信失败", L"WxInitSDK", 0);
        return status;
    }

    if (!IsProcessX64(wcPid)) {
        MessageBox(NULL, L"只支持 64 位微信", L"WxInitSDK", 0);
        return -1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(2)); // 等待微信打开
    wcProcess = InjectDll(wcPid, spyDllPath.c_str(), &spyBase);
    if (wcProcess == NULL) {
        MessageBox(NULL, L"注入失败", L"WxInitSDK", 0);
        return -1;
    }

    PortPath_t pp = { 0 };
    pp.port       = port;
    sprintf_s(pp.path, MAX_PATH, "%s", std::filesystem::current_path().string().c_str());

    if (!CallDllFuncEx(wcProcess, spyDllPath.c_str(), spyBase, "InitSpy", (LPVOID)&pp, sizeof(PortPath_t), NULL)) {
        MessageBox(NULL, L"初始化失败", L"WxInitSDK", 0);
        return -1;
    }

    injected = true;
    return 0;
}

int WxDestroySDK()
{
    if (!injected) {
        return -1;
    }

    if (!CallDllFunc(wcProcess, spyDllPath.c_str(), spyBase, "CleanupSpy", NULL)) {
        return -2;
    }

    if (!EjectDll(wcProcess, spyBase)) {
        return -3; // TODO: Unify error codes
    }

    return 0;
}
