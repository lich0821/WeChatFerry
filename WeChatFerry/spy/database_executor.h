#pragma once

#include <optional>
#include <string>
#include <vector>

#include "wcf.pb.h"

#include "pb_types.h"

namespace db
{

// 获取数据库名称列表
DbNames_t get_db_names();

// 获取指定数据库的表列表
DbTables_t get_db_tables(const std::string &db);

// 执行 SQL 查询
DbRows_t exec_db_query(const std::string &db, const std::string &sql);

// 获取本地消息 ID 和数据库索引
int get_local_id_and_dbidx(uint64_t id, uint64_t *local_id, uint32_t *db_idx);

// 获取音频数据
std::vector<uint8_t> get_audio_data(uint64_t msg_id);

// RPC 方法
bool rpc_get_db_names(uint8_t *out, size_t *len);
bool rpc_get_db_tables(const std::string &db, uint8_t *out, size_t *len);
bool rpc_exec_db_query(const DbQuery query, uint8_t *out, size_t *len);

} // namespace db
