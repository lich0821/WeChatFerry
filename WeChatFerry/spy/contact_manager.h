#pragma once

#include <string>
#include <vector>

#include "pb_types.h"

namespace contact
{

std::vector<RpcContact_t> get_contacts();
int accept_new_friend(const std::string &v3, const std::string &v4, int scene);
int add_friend_by_wxid(const std::string &wxid, const std::string &msg);
RpcContact_t get_contact_by_wxid(const std::string &wxid);

// RPC 函数
bool rpc_get_contacts(uint8_t *out, size_t *len);
bool rpc_get_contact_info(const std::string &wxid, uint8_t *out, size_t *len);
bool rpc_accept_friend(const std::string &v3, const std::string &v4, int scene, uint8_t *out, size_t *len);

} // namespace contact
