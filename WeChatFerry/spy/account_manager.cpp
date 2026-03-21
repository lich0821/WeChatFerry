#include "account_manager.h"

#include <mutex>

#include "log.hpp"
#include "util.h"
#include "offsets.h"
#include "rpc_helper.h"
#include "spy.h"

namespace account
{

namespace OsAcc = Offsets::Account;

// 辅助函数：获取字符串值（x86 平台使用 0x14 偏移）
static std::string get_string_value(uint32_t base_addr, uint32_t offset)
{
    uint32_t type = util::get_dword(base_addr + offset + 0x14);  // x86: 0x14
    if (type == 0xF) {
        return util::get_p_string(base_addr + offset);
    } else {
        return util::get_pp_string(base_addr + offset);
    }
}

std::string get_home_path()
{
    static std::string cached_home;
    static std::once_flag home_once;

    std::call_once(home_once, []() {
        uint32_t base = g_WeChatWinDllAddr;
        if (base) {
            std::string path = util::w2s(util::get_wstring(base + OsAcc::HOME));
            if (!path.empty()) {
                cached_home = path + "\\WeChat Files\\";
            }
        }
    });

    return cached_home;
}

std::string get_self_wxid()
{
    static std::string cached_wxid;
    static std::once_flag wxid_once;

    std::call_once(wxid_once, []() {
        uint32_t base = g_WeChatWinDllAddr;
        if (base) {
            try {
                cached_wxid = get_string_value(base, OsAcc::WXID);
            } catch (...) {
                LOG_ERROR("Failed to get wxid");
                cached_wxid = "empty_wxid";
            }
        }
    });

    return cached_wxid;
}

bool is_logged_in()
{
    uint32_t base = g_WeChatWinDllAddr;
    return base && util::get_dword(base + OsAcc::SERVICE) != 0;
}

UserInfo_t get_user_info()
{
    UserInfo_t ui;
    uint32_t base = g_WeChatWinDllAddr;
    if (!base)
        return ui;

    ui.wxid   = get_self_wxid();
    ui.name   = get_string_value(base, OsAcc::NAME);
    ui.mobile = util::get_p_string(base + OsAcc::MOBILE);
    ui.home   = get_home_path();

    return ui;
}

bool rpc_is_logged_in(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_IS_LOGIN>(out, len, [](Response &rsp) {
        rsp.msg.status = is_logged_in();
    });
}

bool rpc_get_self_wxid(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_SELF_WXID>(out, len, [](Response &rsp) {
        static std::string wxid = get_self_wxid();
        rsp.msg.str = (char *)wxid.c_str();
    });
}

bool rpc_get_user_info(uint8_t *out, size_t *len)
{
    UserInfo_t ui = get_user_info();
    return fill_response<Functions_FUNC_GET_USER_INFO>(out, len, ui, [](Response &rsp, UserInfo_t &ui) {
        rsp.msg.ui.wxid   = (char *)ui.wxid.c_str();
        rsp.msg.ui.name   = (char *)ui.name.c_str();
        rsp.msg.ui.mobile = (char *)ui.mobile.c_str();
        rsp.msg.ui.home   = (char *)ui.home.c_str();
    });
}

} // namespace account
