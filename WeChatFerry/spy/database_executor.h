#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

#include "wcf.pb.h"
#include "pb_types.h"

namespace db
{

DbNames_t get_db_names();
DbTables_t get_db_tables(const std::string &db);
DbRows_t exec_db_query(const std::string &db, const std::string &sql);
int get_local_id_and_dbidx(uint64_t id, uint64_t *local_id, uint32_t *db_idx);
std::vector<uint8_t> get_audio_data(uint64_t msg_id);

// 数据库生密钥：32 字节，返回小写十六进制串，供外部离线解密 .db
std::string get_db_key();

bool rpc_get_db_names(uint8_t *out, size_t *len);
bool rpc_get_db_tables(const std::string &db, uint8_t *out, size_t *len);
bool rpc_exec_db_query(const DbQuery &query, uint8_t *out, size_t *len);
bool rpc_get_db_key(uint8_t *out, size_t *len);

} // namespace db
