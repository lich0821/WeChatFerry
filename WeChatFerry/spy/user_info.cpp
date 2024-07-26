#include "user_info.h"
#include "log.h"
#include "util.h"
#include "wechat_function.h"

extern UINT64 g_WeChatWinDllAddr;

#define OS_USER_HOME   0x5A7E190
#define OS_USER_WXID   0x5AB7F30
#define OS_USER_NAME   0x5AB8098
#define OS_USER_MOBILE 0x5AB7FD8

static char home[MAX_PATH] = { 0 };

string GetHomePath()
{
    if (home[0] == 0) {
<<<<<<< HEAD
        string path = Wstring2String(GET_WSTRING(g_WeChatWinDllAddr + offset::wcf_home)) + "\\WeChat Files\\";
=======
        string path = Wstring2String(GET_WSTRING(g_WeChatWinDllAddr + OS_USER_HOME)) + "\\WeChat Files\\";
>>>>>>> master
        strncpy_s(home, path.c_str(), path.size());
    }

    return string(home);
}

string GetSelfWxid()
{
    UINT64 wxidType = 0;
    try {
<<<<<<< HEAD
        wxidType = GET_UINT64(g_WeChatWinDllAddr + offset::wcf_iwxid + 0x18);
        if (wxidType == 0xF) {
            return GET_STRING_FROM_P(g_WeChatWinDllAddr + offset::wcf_iwxid);
        } else {
            return GET_STRING(g_WeChatWinDllAddr + offset::wcf_iwxid);
        }
    } catch (...) {
        LOG_ERROR("wxid type: {:#x}", wxidType);
        LOG_BUFFER((uint8_t *)(g_WeChatWinDllAddr + offset::wcf_iwxid), 20);
=======
        wxidType = GET_UINT64(g_WeChatWinDllAddr + OS_USER_WXID + 0x18);
        if (wxidType == 0xF) {
            return GET_STRING_FROM_P(g_WeChatWinDllAddr + OS_USER_WXID);
        } else {
            return GET_STRING(g_WeChatWinDllAddr + OS_USER_WXID);
        }
    } catch (...) {
        LOG_ERROR("wxid type: {:#x}", wxidType);
        LOG_BUFFER((uint8_t *)(g_WeChatWinDllAddr + OS_USER_WXID), 20);
>>>>>>> master
        return "empty_wxid";
    }
}

UserInfo_t GetUserInfo()
{
    UserInfo_t ui;

    ui.wxid = GetSelfWxid();

<<<<<<< HEAD
    UINT64 nameType = GET_UINT64(g_WeChatWinDllAddr + offset::wcf_nickName + 0x18);
    if (nameType == 0xF) {
        ui.name = GET_STRING_FROM_P(g_WeChatWinDllAddr + offset::wcf_nickName);
    } else { // 0x1F
        ui.name = GET_STRING(g_WeChatWinDllAddr + offset::wcf_nickName);
    }

    ui.mobile = GET_STRING_FROM_P(g_WeChatWinDllAddr + offset::wcf_mobile);
=======
    UINT64 nameType = GET_UINT64(g_WeChatWinDllAddr + OS_USER_NAME + 0x18);
    if (nameType == 0xF) {
        ui.name = GET_STRING_FROM_P(g_WeChatWinDllAddr + OS_USER_NAME);
    } else { // 0x1F
        ui.name = GET_STRING(g_WeChatWinDllAddr + OS_USER_NAME);
    }

    ui.mobile = GET_STRING_FROM_P(g_WeChatWinDllAddr + OS_USER_MOBILE);
>>>>>>> master
    ui.home   = GetHomePath();

    return ui;
}
