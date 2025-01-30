#include <filesystem>
#include <mutex>

#include "fill_response.h"
#include "log.hpp"
#include "user_info.h"
#include "util.h"

namespace user_info
{

extern UINT64 g_WeChatWinDllAddr;

#define OS_USER_HOME   0x5932770
#define OS_USER_WXID   0x595C270
#define OS_USER_NAME   0x595C3D8
#define OS_USER_MOBILE 0x595C318

std::string get_home_path()
{
    static std::once_flag flag;
    static std::string home_path;

    std::call_once(flag, [] {
        std::string path = Wstring2String(GET_WSTRING(g_WeChatWinDllAddr + OS_USER_HOME)) + "\\WeChat Files\\";
        home_path        = std::filesystem::absolute(path).string();
    });

    return home_path;
}

std::optional<std::string> get_self_wxid()
{
    UINT64 wxid_type = 0;
    try {
        wxid_type = GET_UINT64(g_WeChatWinDllAddr + OS_USER_WXID + 0x18);
        if (wxid_type == 0xF) {
            return GET_STRING_FROM_P(g_WeChatWinDllAddr + OS_USER_WXID);
        } else {
            return GET_STRING(g_WeChatWinDllAddr + OS_USER_WXID);
        }
    } catch (...) {
        LOG_ERROR("Failed to get wxid, type: {:#x}", wxid_type);
        LOG_BUFFER(reinterpret_cast<uint8_t *>(g_WeChatWinDllAddr + OS_USER_WXID), 20);
        return std::nullopt;
    }
}

UserInfo_t get_user_info()
{
    UserInfo_t ui;
    auto wxid = get_self_wxid();
    ui.wxid   = wxid.value_or("unknown_wxid");

    UINT64 name_type = GET_UINT64(g_WeChatWinDllAddr + OS_USER_NAME + 0x18);
    ui.name          = (name_type == 0xF) ? GET_STRING_FROM_P(g_WeChatWinDllAddr + OS_USER_NAME)
                                          : GET_STRING(g_WeChatWinDllAddr + OS_USER_NAME);

    ui.mobile = GET_STRING_FROM_P(g_WeChatWinDllAddr + OS_USER_MOBILE);
    ui.home   = get_home_path();

    return ui;
}

bool rpc_get_self_wxid(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_SELF_WXID>(out, len, [](Response &rsp) {
        auto wxid   = get_self_wxid();
        rsp.msg.str = wxid ? wxid->c_str() : "error";
    });
}

bool rpc_get_user_info(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_USER_INFO>(out, len, [](Response &rsp) {
        UserInfo_t ui     = get_user_info();
        rsp.msg.ui.wxid   = ui.wxid.c_str();
        rsp.msg.ui.name   = ui.name.c_str();
        rsp.msg.ui.mobile = ui.mobile.c_str();
        rsp.msg.ui.home   = ui.home.c_str();
    });
}

} // namespace user_info
