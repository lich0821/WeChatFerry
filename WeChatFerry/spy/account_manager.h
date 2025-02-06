#pragma once

#include <optional>
#include <string>

#include "pb_types.h"

namespace account
{

// 获取 WeChat 数据存储路径
std::string get_home_path();

// 获取自身 wxid
std::string get_self_wxid();

// 获取用户信息
UserInfo_t get_user_info();

// RPC 方法
bool rpc_get_self_wxid(uint8_t *out, size_t *len);
bool rpc_get_user_info(uint8_t *out, size_t *len);

} // namespace account
