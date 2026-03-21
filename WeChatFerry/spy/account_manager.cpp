#include "account_manager.h"

#include <filesystem>

#include "log.hpp"
#include "offsets.h"
#include "rpc_helper.h"
#include "spy.h"
#include "util.h"

namespace account
{

namespace fs    = std::filesystem;
namespace OsAcc = Offsets::Account;

using get_account_service_t = QWORD (*)();
using get_data_path_t       = QWORD (*)(QWORD);

static uint64_t get_account_service()
{
    static auto GetService = Spy::getFunction<get_account_service_t>(OsAcc::SERVICE);
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

fs::path get_home_path()
{
    static fs::path home_path;
    static std::once_flag home_once;

    std::call_once(home_once, []() {
        WxString home {};
        if (auto getDataPath = Spy::getFunction<get_data_path_t>(OsAcc::PATH)) {
            getDataPath(reinterpret_cast<QWORD>(&home));
            if (home.wptr) {
                std::wstring wstr(home.wptr, home.size);
                home_path = util::w2s(std::move(wstr));
            }
        }
    });

    return home_path;
}

std::string get_self_wxid()
{
    static std::string cached_wxid;
    static std::once_flag wxid_once;

    std::call_once(wxid_once, []() {
        if (uint64_t svc = get_account_service(); svc) {
            cached_wxid = get_string_value(svc, OsAcc::WXID);
            if (cached_wxid.empty()) {
                cached_wxid = get_string_value(svc, OsAcc::ALIAS);
            }
        }
    });

    return cached_wxid;
}

UserInfo_t get_user_info()
{
    UserInfo_t ui;
    uint64_t service_addr = get_account_service();
    if (!service_addr) return ui;

    ui.wxid   = get_self_wxid();
    ui.home   = get_home_path().generic_string();
    ui.name   = get_string_value(service_addr, OsAcc::NAME);
    ui.mobile = get_string_value(service_addr, OsAcc::MOBILE);
    ui.alias  = get_string_value(service_addr, OsAcc::ALIAS);
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
        rsp.msg.ui.alias  = (char *)ui.alias.c_str();
    });
}

} // namespace account
