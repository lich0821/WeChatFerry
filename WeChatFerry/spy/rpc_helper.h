#pragma once

#include <magic_enum/magic_enum.hpp>
#include <unordered_map>

#include "wcf.pb.h"

#include "log.hpp"
#include "pb_encode.h"
#include "pb_types.h"

using FunctionHandler = std::function<bool(const Request &, uint8_t *, size_t *)>;

static const std::unordered_map<Functions, int> rpc_tag_map
    = { { Functions_FUNC_IS_LOGIN, Response_status_tag },
        { Functions_FUNC_GET_SELF_WXID, Response_str_tag },
        { Functions_FUNC_GET_USER_INFO, Response_ui_tag },
        { Functions_FUNC_GET_MSG_TYPES, Response_types_tag },
        { Functions_FUNC_GET_CONTACTS, Response_contacts_tag },
        { Functions_FUNC_GET_DB_NAMES, Response_dbs_tag },
        { Functions_FUNC_GET_DB_TABLES, Response_tables_tag },
        { Functions_FUNC_GET_AUDIO_MSG, Response_str_tag },
        { Functions_FUNC_SEND_TXT, Response_status_tag },
        { Functions_FUNC_SEND_IMG, Response_status_tag },
        { Functions_FUNC_SEND_FILE, Response_status_tag },
        { Functions_FUNC_SEND_RICH_TXT, Response_status_tag },
        { Functions_FUNC_SEND_PAT_MSG, Response_status_tag },
        { Functions_FUNC_FORWARD_MSG, Response_status_tag },
        { Functions_FUNC_SEND_EMOTION, Response_status_tag },
        { Functions_FUNC_ENABLE_RECV_TXT, Response_status_tag },
        { Functions_FUNC_DISABLE_RECV_TXT, Response_status_tag },
        { Functions_FUNC_EXEC_DB_QUERY, Response_rows_tag },
        { Functions_FUNC_REFRESH_PYQ, Response_status_tag },
        { Functions_FUNC_DOWNLOAD_ATTACH, Response_status_tag },
        { Functions_FUNC_RECV_TRANSFER, Response_status_tag },
        { Functions_FUNC_REVOKE_MSG, Response_status_tag },
        { Functions_FUNC_REFRESH_QRCODE, Response_str_tag },
        { Functions_FUNC_DECRYPT_IMAGE, Response_str_tag },
        { Functions_FUNC_EXEC_OCR, Response_ocr_tag },
        { Functions_FUNC_ADD_ROOM_MEMBERS, Response_status_tag },
        { Functions_FUNC_DEL_ROOM_MEMBERS, Response_status_tag },
        { Functions_FUNC_INV_ROOM_MEMBERS, Response_status_tag } };

const std::unordered_map<Functions, FunctionHandler> rpc_function_map = {
    { Functions_FUNC_IS_LOGIN, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_GET_SELF_WXID, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_GET_USER_INFO, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_GET_MSG_TYPES, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_GET_CONTACTS, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_GET_DB_NAMES, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_GET_DB_TABLES, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_GET_AUDIO_MSG, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_SEND_TXT, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_SEND_IMG, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_SEND_FILE, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_SEND_RICH_TXT, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_SEND_PAT_MSG, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_FORWARD_MSG, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_SEND_EMOTION, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_ENABLE_RECV_TXT, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_DISABLE_RECV_TXT, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_EXEC_DB_QUERY, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_REFRESH_PYQ, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_DOWNLOAD_ATTACH, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_RECV_TRANSFER, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_REVOKE_MSG, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_REFRESH_QRCODE, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_DECRYPT_IMAGE, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_EXEC_OCR, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_ADD_ROOM_MEMBERS, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_DEL_ROOM_MEMBERS, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    { Functions_FUNC_INV_ROOM_MEMBERS, [](const Request &r, uint8_t *out, size_t *out_len) { return; } },
    // clang-format off
    // clang-format on
};

template <Functions FuncType, typename AssignFunc> bool fill_response(uint8_t *out, size_t *len, AssignFunc assign)
{
    Response rsp = Response_init_default;
    rsp.func     = FuncType;

    auto it = rpc_tag_map.find(FuncType);
    if (it == rpc_tag_map.end()) {
        LOG_ERROR("Unknown function type: {}", magic_enum::enum_name(rsp.func));
        return false;
    }
    rsp.which_msg = it->second;

    assign(rsp);

    pb_ostream_t stream = pb_ostream_from_buffer(out, *len);
    if (!pb_encode(&stream, Response_fields, &rsp)) {
        LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(&stream));
        return false;
    }
    *len = stream.bytes_written;
    return true;
}
