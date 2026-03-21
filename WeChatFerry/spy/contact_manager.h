#pragma once

#include <string>
#include <vector>

#include "wcf.pb.h"

#include "pb_types.h"

namespace contact
{

// 获取所有联系人
std::vector<RpcContact_t> get_contacts();

// 根据 wxid 获取联系人信息
RpcContact_t get_contact_by_wxid(const std::string &wxid);

// 接受好友请求
int accept_new_friend(const std::string &v3, const std::string &v4, int scene);

// 发送好友请求
// int add_friend_by_wxid(const std::string &wxid, const std::string &msg);

// RPC 方法
bool rpc_get_contacts(uint8_t *out, size_t *len);
bool rpc_get_contact_info(const std::string &wxid, uint8_t *out, size_t *len);
bool rpc_accept_friend(const Verification &v, uint8_t *out, size_t *len);

} // namespace contact
