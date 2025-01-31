#pragma once

#include <string>

namespace chatroom_mgmt
{

// 添加成员到群聊
int add_chatroom_member(const std::string &roomid, const std::string &wxids);

// 从群聊中移除成员
int del_chatroom_member(const std::string &roomid, const std::string &wxids);

// 邀请成员加入群聊
int invite_chatroom_member(const std::string &roomid, const std::string &wxids);

// RPC 方法
bool rpc_add_chatroom_member(const std::string &roomid, const std::string &wxids, uint8_t *out, size_t *len);
bool rpc_del_chatroom_member(const std::string &roomid, const std::string &wxids, uint8_t *out, size_t *len);
bool rpc_invite_chatroom_member(const std::string &roomid, const std::string &wxids, uint8_t *out, size_t *len);

} // namespace chatroom_mgmt
