#include "user_info.h"
#include "load_calls.h"
#include "log.h"
#include "util.h"

extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

static char home[MAX_PATH] = { 0 };

string GetHomePath()
{
    if (home[0] == 0) {
        string path = Wstring2String(GET_WSTRING(g_WeChatWinDllAddr + g_WxCalls.ui.home)) + "\\WeChat Files\\";
        strncpy_s(home, path.c_str(), path.size());
    }

    return string(home);
}

string GetSelfWxid()
{
    DWORD wxidType = 0;
    try {
        wxidType = GET_DWORD(g_WeChatWinDllAddr + g_WxCalls.ui.wxid + 0x14);
        if (wxidType == 0xF) {
            return GET_STRING_FROM_P(g_WeChatWinDllAddr + g_WxCalls.ui.wxid);
        } else {
            return GET_STRING(g_WeChatWinDllAddr + g_WxCalls.ui.wxid);
        }
    } catch (...) {
        LOG_ERROR("wxid type: {:#x}", wxidType);
        LOG_BUFFER((uint8_t *)(g_WeChatWinDllAddr + g_WxCalls.ui.wxid), 20);
        return "empty_wxid";
    }
}

UserInfo_t GetUserInfo()
{
    UserInfo_t ui;

    ui.wxid = GetSelfWxid();

    DWORD nameType = GET_DWORD(g_WeChatWinDllAddr + g_WxCalls.ui.nickName + 0x14);
    if (nameType == 0xF) {
        ui.name = GET_STRING_FROM_P(g_WeChatWinDllAddr + g_WxCalls.ui.nickName);
    } else { // 0x1F
        ui.name = GET_STRING(g_WeChatWinDllAddr + g_WxCalls.ui.nickName);
    }

    ui.mobile = GET_STRING_FROM_P(g_WeChatWinDllAddr + g_WxCalls.ui.mobile);
    ui.home   = GetHomePath();

    return ui;
}
