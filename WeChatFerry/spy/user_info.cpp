#include "user_info.h"
#include "load_calls.h"
#include "log.h"
#include "util.h"
#include "wechat_function.h"

extern WxCalls_t g_WxCalls;
extern UINT64 g_WeChatWinDllAddr;

static char home[MAX_PATH] = { 0 };

string GetHomePath()
{
    if (home[0] == 0) {
        string path = Wstring2String(GET_WSTRING(g_WeChatWinDllAddr + offset::wcf_home)) + "\\WeChat Files\\";
        strncpy_s(home, path.c_str(), path.size());
    }

    return string(home);
}

string GetSelfWxid()
{
    UINT64 wxidType = 0;
    try {
        wxidType = GET_UINT64(g_WeChatWinDllAddr + offset::wcf_iwxid + 0x18);
        if (wxidType == 0xF) {
            return GET_STRING_FROM_P(g_WeChatWinDllAddr + offset::wcf_iwxid);
        } else {
            return GET_STRING(g_WeChatWinDllAddr + offset::wcf_iwxid);
        }
    } catch (...) {
        LOG_ERROR("wxid type: {:#x}", wxidType);
        LOG_BUFFER((uint8_t *)(g_WeChatWinDllAddr + offset::wcf_iwxid), 20);
        return "empty_wxid";
    }
}

UserInfo_t GetUserInfo()
{
    UserInfo_t ui;

    ui.wxid = GetSelfWxid();

    UINT64 nameType = GET_UINT64(g_WeChatWinDllAddr + offset::wcf_nickName + 0x18);
    if (nameType == 0xF) {
        ui.name = GET_STRING_FROM_P(g_WeChatWinDllAddr + offset::wcf_nickName);
    } else { // 0x1F
        ui.name = GET_STRING(g_WeChatWinDllAddr + offset::wcf_nickName);
    }

    ui.mobile = GET_STRING_FROM_P(g_WeChatWinDllAddr + offset::wcf_mobile);
    ui.home   = GetHomePath();

    return ui;
}
