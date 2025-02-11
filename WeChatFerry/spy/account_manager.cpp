#include "account_manager.h"

#include <filesystem>

#include "log.hpp"
#include "offsets.h"
#include "rpc_helper.h"
#include "util.h"

extern UINT64 g_WeChatWinDllAddr;

namespace account
{

namespace OsAcc = Offsets::Account;

using get_account_service_t   = QWORD (*)();
using get_current_data_path_t = QWORD (*)(QWORD);

static uint64_t get_account_service()
{
    static auto GetService = reinterpret_cast<get_account_service_t>(g_WeChatWinDllAddr + OsAcc::SERVICE);
    return GetService ? GetService() : 0;
}

static std::string get_string_value(uint64_t base_addr, uint64_t offset)
{
    uint64_t type = util::get_qword(base_addr + offset + 0x18);
    return (type == 0xF) ? util::get_p_string(base_addr + offset) : util::get_pp_string(base_addr + offset);
}

bool is_logged_in()
{
    uint64_t service_addr = get_account_service();
    return service_addr && util::get_qword(service_addr + OsAcc::LOGIN) != 0;
}

std::string get_self_wxid()
{
    uint64_t service_addr = get_account_service();
    if (!service_addr) return "";

    return get_string_value(service_addr, OsAcc::WXID);
}

UserInfo_t get_user_info()
{
    UserInfo_t ui;
    WxString home;

    auto GetDataPath = reinterpret_cast<get_current_data_path_t>(g_WeChatWinDllAddr + OsAcc::PATH);

    uint64_t service_addr = get_account_service();
    if (!service_addr) return ui;

    ui.wxid = get_self_wxid();
    GetDataPath((QWORD)&home);
    if (home.wptr) {
        std::filesystem::path path = util::w2s(std::wstring(home.wptr, home.size));
        ui.home                    = path.generic_string();
    }
    ui.name   = get_string_value(service_addr, OsAcc::NAME);
    ui.mobile = get_string_value(service_addr, OsAcc::MOBILE);
    return ui;
}

bool rpc_is_logged_in(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_IS_LOGIN>(out, len, [](Response &rsp) { rsp.msg.status = is_logged_in(); });
}

bool rpc_get_self_wxid(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_SELF_WXID>(
        out, len, [](Response &rsp) { rsp.msg.str = (char *)get_self_wxid().c_str(); });
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
