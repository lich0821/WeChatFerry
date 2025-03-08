#include "sdk.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <optional>
#include <process.h>
#include <sstream>
#include <thread>

#include "framework.h"
#include <tlhelp32.h>

#include "injector.h"
#include "util.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

static bool injected    = false;
static HANDLE wcProcess = NULL;
static HMODULE spyBase  = NULL;
static std::string spyDllPath;

constexpr char WCFSDKDLL[]       = "sdk.dll";
constexpr char WCFSPYDLL[]       = "spy.dll";
constexpr char WCFSPYDLL_DEBUG[] = "spy_debug.dll";

constexpr std::string_view DISCLAIMER_FLAG      = ".license_accepted.flag";
constexpr std::string_view DISCLAIMER_TEXT_FILE = "DISCLAIMER.md";

namespace fs = std::filesystem;

static fs::path get_module_directory()
{
    char buffer[MAX_PATH] = { 0 };
    HMODULE hModule       = reinterpret_cast<HMODULE>(&__ImageBase);
    GetModuleFileNameA(hModule, buffer, MAX_PATH);
    fs::path modulePath(buffer);
    return modulePath.parent_path();
}

static bool show_disclaimer()
{
    fs::path sdk_path = get_module_directory();
    if (fs::exists(sdk_path / DISCLAIMER_FLAG)) {
        return true;
    }

    fs::path path = sdk_path / DISCLAIMER_TEXT_FILE;
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        util::MsgBox(NULL, "免责声明文件读取失败。", "错误", MB_ICONERROR);
        return false;
    }

    auto disclaimerText = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (disclaimerText.empty()) {
        util::MsgBox(NULL, "免责声明文件为空", "错误", MB_ICONERROR);
        return false;
    }

    int result = util::MsgBox(NULL, disclaimerText.c_str(), "免责声明", MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2);
    if (result == IDCANCEL) {
        util::MsgBox(NULL, "您拒绝了免责声明，程序将退出。", "提示", MB_ICONINFORMATION);
        return false;
    }

    std::ofstream flagFile(sdk_path / DISCLAIMER_FLAG, std::ios::out | std::ios::trunc);
    if (!flagFile) {
        util::MsgBox(NULL, "无法创建协议标志文件。", "错误", MB_ICONERROR);
        return false;
    }
    flagFile << "User accepted the license agreement.";

    return true;
}

static std::string get_dll_path(bool debug)
{
    char buffer[MAX_PATH] = { 0 };
    GetModuleFileNameA(GetModuleHandleA(WCFSDKDLL), buffer, MAX_PATH);

    fs::path path(buffer);
    path.remove_filename(); // 只保留目录路径
    path /= debug ? WCFSPYDLL_DEBUG : WCFSPYDLL;

    if (!fs::exists(path)) {
        util::MsgBox(NULL, path.string().c_str(), "文件不存在", MB_ICONERROR);
        return "";
    }

    return path.string();
}

int WxInitSDK(bool debug, int port)
{
    if (!show_disclaimer()) {
        exit(-1); // 用户拒绝协议，退出程序
    }

    int status  = 0;
    DWORD wcPid = 0;

    spyDllPath = get_dll_path(debug);
    if (spyDllPath.empty()) {
        return ERROR_FILE_NOT_FOUND; // DLL 文件路径不存在
    }

    status = util::open_wechat(wcPid);
    if (status != 0) {
        util::MsgBox(NULL, "打开微信失败", "WxInitSDK", 0);
        return status;
    }

    std::this_thread::sleep_for(std::chrono::seconds(2)); // 等待微信打开
    wcProcess = inject_dll(wcPid, spyDllPath, &spyBase);
    if (wcProcess == NULL) {
        util::MsgBox(NULL, "注入失败", "WxInitSDK", 0);
        return -1;
    }
    injected = true;

    util::PortPath pp = { 0 };
    pp.port           = port;
    snprintf(pp.path, MAX_PATH, "%s", fs::current_path().string().c_str());

    status       = -3; // TODO: 统一错误码
    bool success = call_dll_func_ex(wcProcess, spyDllPath, spyBase, "InitSpy", (LPVOID)&pp, sizeof(util::PortPath),
                                    (DWORD *)&status);
    if (!success || status != 0) {
        WxDestroySDK();
    }

    return status;
}

int WxDestroySDK()
{
    if (!injected) {
        return 1; // 未注入
    }

    if (!call_dll_func(wcProcess, spyDllPath, spyBase, "CleanupSpy", NULL)) {
        return -1;
    }

    if (!eject_dll(wcProcess, spyBase)) {
        return -2;
    }
    injected = false;

    return 0;
}
