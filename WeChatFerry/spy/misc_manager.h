#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

#include "wcf.pb.h"
#include "pb_types.h"

namespace misc
{

// 获取音频消息
std::string get_audio(uint64_t id, const std::string &dir);

// 解密图片
std::string decrypt_image(const std::string &src, const std::string &dst);

// 刷新朋友圈
int refresh_pyq(uint64_t id);

// 下载附件
int download_attachment(uint64_t id, const std::string &thumb, const std::string &extra);

// 撤回消息
int revoke_message(uint64_t id);

// OCR 识别
OcrResult_t get_ocr_result(const std::string &path);

// 获取登录二维码
std::string get_login_url();

// 接收转账
int receive_transfer(const std::string &wxid, const std::string &transferid, const std::string &transactionid);

// RPC 方法
bool rpc_get_audio(const AudioMsg &am, uint8_t *out, size_t *len);
bool rpc_decrypt_image(const DecPath &dec, uint8_t *out, size_t *len);
bool rpc_refresh_pyq(uint64_t id, uint8_t *out, size_t *len);
bool rpc_download_attachment(const AttachMsg &att, uint8_t *out, size_t *len);
bool rpc_revoke_message(uint64_t id, uint8_t *out, size_t *len);
bool rpc_get_ocr_result(const std::string &path, uint8_t *out, size_t *len);
bool rpc_get_login_url(uint8_t *out, size_t *len);
bool rpc_receive_transfer(const Transfer &tf, uint8_t *out, size_t *len);

} // namespace misc
