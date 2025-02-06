#include "account_manager.h"

#include <filesystem>
#include <mutex>

#include "log.hpp"
#include "rpc_helper.h"
#include "util.h"

extern UINT64 g_WeChatWinDllAddr;

namespace account
{
#define OS_USER_HOME   0x5932770
#define OS_USER_WXID   0x595C270
#define OS_USER_NAME   0x595C3D8
#define OS_USER_MOBILE 0x595C318

std::string get_home_path()
{
    static std::once_flag flag;
    static std::string home_path;

    std::call_once(flag, [] {
        std::string path = util::w2s(util::get_pp_wstring(g_WeChatWinDllAddr + OS_USER_HOME)) + "\\WeChat Files\\";
        home_path        = std::filesystem::absolute(path).string();
    });

    return home_path;
}

std::string get_self_wxid()
{
    static std::once_flag flag;
    static std::string wxid;

    std::call_once(flag, [] {
        UINT64 wxid_type = 0;
        try {
            wxid_type = util::get_qword(g_WeChatWinDllAddr + OS_USER_WXID + 0x18);
            if (wxid_type == 0xF) {
                wxid = util::get_p_string(g_WeChatWinDllAddr + OS_USER_WXID);
            } else {
                wxid = util::get_pp_string(g_WeChatWinDllAddr + OS_USER_WXID);
            }

        } catch (...) {
            LOG_ERROR("Failed to get wxid, type: {:#x}", wxid_type);
            LOG_BUFFER(reinterpret_cast<uint8_t *>(g_WeChatWinDllAddr + OS_USER_WXID), 20);
            wxid = "获取wxid失败";
        }
    });
    return wxid;
}

UserInfo_t get_user_info()
{
    UserInfo_t ui;
    ui.wxid = get_self_wxid();

    UINT64 name_type = util::get_qword(g_WeChatWinDllAddr + OS_USER_NAME + 0x18);
    ui.name          = (name_type == 0xF) ? util::get_p_string(g_WeChatWinDllAddr + OS_USER_NAME)
                                          : util::get_pp_string(g_WeChatWinDllAddr + OS_USER_NAME);

    ui.mobile = util::get_p_string(g_WeChatWinDllAddr + OS_USER_MOBILE);
    ui.home   = get_home_path();

    return ui;
}

bool rpc_get_self_wxid(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_SELF_WXID>(
        out, len, [](Response &rsp) { rsp.msg.str = (char *)get_self_wxid().c_str(); });
}

bool rpc_get_user_info(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_USER_INFO>(out, len, [](Response &rsp) {
        UserInfo_t ui     = get_user_info();
        rsp.msg.ui.wxid   = (char *)ui.wxid.c_str();
        rsp.msg.ui.name   = (char *)ui.name.c_str();
        rsp.msg.ui.mobile = (char *)ui.mobile.c_str();
        rsp.msg.ui.home   = (char *)ui.home.c_str();
    });
}

} // namespace account
