#pragma once

#include "string"
#include <vector>

#include "pb_types.h"

vector<RpcContact_t> GetContacts();
int AcceptNewFriend(std::string v3, std::string v4, int scene);
int AddFriendByWxid(std::string wxid, std::string msg);
RpcContact_t GetContactByWxid(std::string wxid);
