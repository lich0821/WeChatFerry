#include "user_info.h"
#include "load_calls.h"
#include "log.h"
#include "util.h"

extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

string GetHomePath() { return GET_STRING(g_WeChatWinDllAddr + g_WxCalls.ui.home); }

string GetSelfWxid()
{
    DWORD wxidType = 0;
    try {
        wxidType = GET_DWORD(g_WeChatWinDllAddr + g_WxCalls.ui.wxid + 0x14);
        LOG_DEBUG("WeChatWinDll: {:#x}, wxid type: {:#x}", g_WeChatWinDllAddr, wxidType);
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

    ui.wxid   = GetSelfWxid();
    ui.name   = GET_STRING_FROM_P(g_WeChatWinDllAddr + g_WxCalls.ui.nickName);
    ui.mobile = GET_STRING_FROM_P(g_WeChatWinDllAddr + g_WxCalls.ui.mobile);
    ui.home   = GET_STRING(g_WeChatWinDllAddr + g_WxCalls.ui.home);

    return ui;
}
