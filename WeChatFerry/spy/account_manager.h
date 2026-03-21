#pragma once

#include <string>

#include "pb_types.h"

namespace account
{

std::string get_home_path();
std::string get_self_wxid();
bool is_logged_in();
UserInfo_t get_user_info();

// RPC 函数
bool rpc_is_logged_in(uint8_t *out, size_t *len);
bool rpc_get_self_wxid(uint8_t *out, size_t *len);
bool rpc_get_user_info(uint8_t *out, size_t *len);

} // namespace account
