#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "wcf.pb.h"

#include "pb_types.h"

namespace misc
{

std::string get_audio(uint64_t id, const std::filesystem::path &dir);
std::string get_pcm_audio(uint64_t id, const std::filesystem::path &dir, int32_t sr);
std::string decrypt_image(const std::filesystem::path &src, const std::filesystem::path &dst);
std::string get_login_url();

int refresh_pyq(uint64_t id);
int download_attachment(uint64_t id, const std::filesystem::path &thumb, const std::filesystem::path &extra);
int revoke_message(uint64_t id);

OcrResult_t get_ocr_result(const std::filesystem::path &path);
int receive_transfer(const std::string &wxid, const std::string &transferid, const std::string &transactionid);

// RPC
// clang-format off
bool rpc_get_audio(const AudioMsg &am, uint8_t *out, size_t *len);
bool rpc_get_pcm_audio(uint64_t id, const std::filesystem::path &dir, int32_t sr, uint8_t *out, size_t *len);
bool rpc_decrypt_image(const DecPath &dec, uint8_t *out, size_t *len);
bool rpc_get_login_url(uint8_t *out, size_t *len);
bool rpc_refresh_pyq(uint64_t id, uint8_t *out, size_t *len);
bool rpc_download_attachment(const AttachMsg &att, uint8_t *out, size_t *len);
bool rpc_revoke_message(uint64_t id, uint8_t *out, size_t *len);
bool rpc_get_ocr_result(const std::filesystem::path &path, uint8_t *out, size_t *len);
bool rpc_receive_transfer(const Transfer &tf, uint8_t *out, size_t *len);
bool rpc_shutdown(uint8_t *out, size_t *len);
// clang-format on
} // namespace misc
