#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

#include "wcf.pb.h"

namespace chatroom
{

int add_chatroom_member(const std::string &roomid, const std::string &wxids);
int del_chatroom_member(const std::string &roomid, const std::string &wxids);
int invite_chatroom_member(const std::string &roomid, const std::string &wxids);

bool rpc_add_chatroom_member(const MemberMgmt &m, uint8_t *out, size_t *len);
bool rpc_delete_chatroom_member(const MemberMgmt &m, uint8_t *out, size_t *len);
bool rpc_invite_chatroom_member(const MemberMgmt &m, uint8_t *out, size_t *len);

} // namespace chatroom
