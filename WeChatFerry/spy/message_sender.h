#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

#include "wcf.pb.h"

namespace message
{

// 发送文本消息
void send_text(const std::string &wxid, const std::string &msg, const std::string &at_wxids = "");

// 发送图片消息
void send_image(const std::string &wxid, const std::string &path);

// 发送文件消息
void send_file(const std::string &wxid, const std::string &path);

// 发送 XML 消息
void send_xml(const std::string &receiver, const std::string &xml, const std::string &path, int type);

// 发送表情消息
void send_emotion(const std::string &wxid, const std::string &path);

// 发送富文本消息
int send_rich_text(const RichText &rt);

// 发送拍一拍消息
int send_pat(const std::string &roomid, const std::string &wxid);

// 转发消息
int forward(uint64_t msgid, const std::string &receiver);

// RPC 方法
bool rpc_send_text(const TextMsg &text, uint8_t *out, size_t *len);
bool rpc_send_image(const PathMsg &file, uint8_t *out, size_t *len);
bool rpc_send_file(const PathMsg &file, uint8_t *out, size_t *len);
bool rpc_send_emotion(const PathMsg &file, uint8_t *out, size_t *len);
bool rpc_send_xml(const XmlMsg &xml, uint8_t *out, size_t *len);
bool rpc_send_rich_text(const RichText &rt, uint8_t *out, size_t *len);
bool rpc_send_pat(const PatMsg &pat, uint8_t *out, size_t *len);
bool rpc_forward(const ForwardMsg &fm, uint8_t *out, size_t *len);

} // namespace message
