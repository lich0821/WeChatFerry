#include "user_info.h"
#include "log.h"
#include "util.h"

extern UINT64 g_WeChatWinDllAddr;

#define OS_USER_HOME   0x5932770
#define OS_USER_WXID   0x595C270
#define OS_USER_NAME   0x595C3D8
#define OS_USER_MOBILE 0x595C318

static char home[MAX_PATH] = { 0 };

string GetHomePath()
{
    if (home[0] == 0) {
        string path = Wstring2String(GET_WSTRING(g_WeChatWinDllAddr + OS_USER_HOME)) + "\\WeChat Files\\";
        strncpy_s(home, path.c_str(), path.size());
    }

    return string(home);
}

string GetSelfWxid()
{
    UINT64 wxidType = 0;
    try {
        wxidType = GET_UINT64(g_WeChatWinDllAddr + OS_USER_WXID + 0x18);
        if (wxidType == 0xF) {
            return GET_STRING_FROM_P(g_WeChatWinDllAddr + OS_USER_WXID);
        } else {
            return GET_STRING(g_WeChatWinDllAddr + OS_USER_WXID);
        }
    } catch (...) {
        LOG_ERROR("wxid type: {:#x}", wxidType);
        LOG_BUFFER((uint8_t *)(g_WeChatWinDllAddr + OS_USER_WXID), 20);
        return "empty_wxid";
    }
}

UserInfo_t GetUserInfo()
{
    UserInfo_t ui;

    ui.wxid = GetSelfWxid();

    UINT64 nameType = GET_UINT64(g_WeChatWinDllAddr + OS_USER_NAME + 0x18);
    if (nameType == 0xF) {
        ui.name = GET_STRING_FROM_P(g_WeChatWinDllAddr + OS_USER_NAME);
    } else { // 0x1F
        ui.name = GET_STRING(g_WeChatWinDllAddr + OS_USER_NAME);
    }

    ui.mobile = GET_STRING_FROM_P(g_WeChatWinDllAddr + OS_USER_MOBILE);
    ui.home   = GetHomePath();

    return ui;
}
