#pragma once

#include <unordered_map>

#include "wcf.pb.h"
#include "pb_encode.h"
#include "pb_types.h"
#include "log.hpp"

// RPC 函数类型到响应标签的映射表
static const std::unordered_map<Functions, int> rpc_tag_map = {
    { Functions_FUNC_IS_LOGIN, Response_status_tag },
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
    { Functions_FUNC_SEND_XML, Response_status_tag },
    { Functions_FUNC_SEND_RICH_TXT, Response_status_tag },
    { Functions_FUNC_SEND_PAT_MSG, Response_status_tag },
    { Functions_FUNC_FORWARD_MSG, Response_status_tag },
    { Functions_FUNC_SEND_EMOTION, Response_status_tag },
    { Functions_FUNC_ENABLE_RECV_TXT, Response_status_tag },
    { Functions_FUNC_DISABLE_RECV_TXT, Response_status_tag },
    { Functions_FUNC_EXEC_DB_QUERY, Response_rows_tag },
    { Functions_FUNC_REFRESH_PYQ, Response_status_tag },
    { Functions_FUNC_DOWNLOAD_ATTACH, Response_status_tag },
    { Functions_FUNC_GET_CONTACT_INFO, Response_contacts_tag },
    { Functions_FUNC_ACCEPT_FRIEND, Response_status_tag },
    { Functions_FUNC_RECV_TRANSFER, Response_status_tag },
    { Functions_FUNC_REVOKE_MSG, Response_status_tag },
    { Functions_FUNC_REFRESH_QRCODE, Response_str_tag },
    { Functions_FUNC_DECRYPT_IMAGE, Response_str_tag },
    { Functions_FUNC_EXEC_OCR, Response_ocr_tag },
    { Functions_FUNC_ADD_ROOM_MEMBERS, Response_status_tag },
    { Functions_FUNC_DEL_ROOM_MEMBERS, Response_status_tag },
    { Functions_FUNC_INV_ROOM_MEMBERS, Response_status_tag }
};

// 模板函数：简化 RPC 响应填充（无额外数据）
template <Functions FuncType, typename AssignFunc>
bool fill_response(uint8_t *out, size_t *len, AssignFunc assign)
{
    Response rsp = Response_init_default;
    rsp.func     = FuncType;

    auto it = rpc_tag_map.find(FuncType);
    if (it == rpc_tag_map.end()) {
        LOG_ERROR("Unknown function type: %d", static_cast<int>(FuncType));
        return false;
    }
    rsp.which_msg = it->second;

    assign(rsp);

    pb_ostream_t stream = pb_ostream_from_buffer(out, *len);
    if (!pb_encode(&stream, Response_fields, &rsp)) {
        LOG_ERROR("Encoding failed: %s", PB_GET_ERROR(&stream));
        return false;
    }
    *len = stream.bytes_written;
    return true;
}

// 模板函数：简化 RPC 响应填充（带额外数据）
template <Functions FuncType, typename DataType, typename AssignFunc>
bool fill_response(uint8_t *out, size_t *len, DataType &&data, AssignFunc &&assign)
{
    Response rsp = Response_init_default;
    rsp.func     = FuncType;

    auto it = rpc_tag_map.find(FuncType);
    if (it == rpc_tag_map.end()) {
        LOG_ERROR("Unknown function type: %d", static_cast<int>(FuncType));
        return false;
    }
    rsp.which_msg = it->second;

    assign(rsp, data);

    pb_ostream_t stream = pb_ostream_from_buffer(out, *len);
    if (!pb_encode(&stream, Response_fields, &rsp)) {
        LOG_ERROR("Encoding failed: %s", PB_GET_ERROR(&stream));
        return false;
    }
    *len = stream.bytes_written;

    return true;
}
