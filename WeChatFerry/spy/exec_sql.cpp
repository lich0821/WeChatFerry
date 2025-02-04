#include "exec_sql.h"

#include <algorithm>
#include <iterator>

#include "fill_response.h"
#include "log.hpp"
#include "pb_util.h"
#include "sqlite3.h"
#include "util.h"

extern UINT64 g_WeChatWinDllAddr;

namespace exec_sql
{
#define OFFSET_DB_INSTANCE     0x5902000
#define OFFSET_DB_MICROMSG     0xB8
#define OFFSET_DB_CHAT_MSG     0x2C8
#define OFFSET_DB_MISC         0x5F0
#define OFFSET_DB_EMOTION      0x15F0
#define OFFSET_DB_MEDIA        0xF48
#define OFFSET_DB_BIZCHAT_MSG  0x1A70
#define OFFSET_DB_FUNCTION_MSG 0x1B98
#define OFFSET_DB_NAME         0x28
#define OFFSET_DB_MSG_MGR      0x595F900

using db_map_t = std::map<std::string, QWORD>;
static db_map_t db_map;

static void get_db_handle(QWORD base, QWORD offset)
{
    auto *wsp          = reinterpret_cast<wchar_t *>(*(QWORD *)(base + offset + OFFSET_DB_NAME));
    std::string dbname = util::w2s(std::wstring(wsp));
    db_map[dbname]     = util::get_qword(base + offset);
}

static void get_msg_db_handle(QWORD msg_mgr_addr)
{
    QWORD db_index = util::get_qword(msg_mgr_addr + 0x68);
    QWORD p_start  = util::get_qword(msg_mgr_addr + 0x50);
    for (uint32_t i = 0; i < db_index; i++) {
        QWORD db_addr = util::get_qword(p_start + i * 0x08);
        if (db_addr) {
            // MSGi.db
            std::string dbname = util::w2s(util::get_pp_wstring(db_addr));
            db_map[dbname]     = util::get_qword(db_addr + 0x78);

            // MediaMsgi.db
            QWORD mmdb_addr      = util::get_qword(db_addr + 0x20);
            std::string mmdbname = util::w2s(util::get_pp_wstring(mmdb_addr + 0x78));
            db_map[mmdbname]     = util::get_qword(mmdb_addr + 0x50);
        }
    }
}

db_map_t get_db_handles()
{
    db_map.clear();
    QWORD db_instance_addr = util::get_qword(g_WeChatWinDllAddr + OFFSET_DB_INSTANCE);

    get_db_handle(db_instance_addr, OFFSET_DB_MICROMSG);     // MicroMsg.db
    get_db_handle(db_instance_addr, OFFSET_DB_CHAT_MSG);     // ChatMsg.db
    get_db_handle(db_instance_addr, OFFSET_DB_MISC);         // Misc.db
    get_db_handle(db_instance_addr, OFFSET_DB_EMOTION);      // Emotion.db
    get_db_handle(db_instance_addr, OFFSET_DB_MEDIA);        // Media.db
    get_db_handle(db_instance_addr, OFFSET_DB_FUNCTION_MSG); // Function.db

    get_msg_db_handle(util::get_qword(g_WeChatWinDllAddr + OFFSET_DB_MSG_MGR)); // MSGi.db & MediaMsgi.db

    return db_map;
}

DbNames_t get_db_names()
{
    if (db_map.empty()) {
        db_map = get_db_handles();
    }

    DbNames_t names;
    for (const auto &[k, _] : db_map) {
        names.push_back(k);
    }
    return names;
}

static int cb_get_tables(void *ret, int argc, char **argv, char **azColName)
{
    auto *tables = static_cast<DbTables_t *>(ret);
    DbTable_t tbl;
    for (int i = 0; i < argc; i++) {
        if (strcmp(azColName[i], "name") == 0) {
            tbl.name = argv[i] ? argv[i] : "";
        } else if (strcmp(azColName[i], "sql") == 0) {
            std::string sql(argv[i]);
            sql.erase(std::remove(sql.begin(), sql.end(), '\t'), sql.end());
            tbl.sql = sql;
        }
    }
    tables->push_back(tbl);
    return 0;
}

DbTables_t get_db_tables(const std::string &db)
{
    DbTables_t tables;
    if (db_map.empty()) {
        db_map = get_db_handles();
    }

    auto it = db_map.find(db);
    if (it == db_map.end()) {
        return tables;
    }

    constexpr const char *sql   = "SELECT name FROM sqlite_master WHERE type='table';";
    Sqlite3_exec p_sqlite3_exec = reinterpret_cast<Sqlite3_exec>(g_WeChatWinDllAddr + SQLITE3_EXEC_OFFSET);
    p_sqlite3_exec(it->second, sql, (Sqlite3_callback)cb_get_tables, (void *)&tables, nullptr);

    return tables;
}

DbRows_t exec_db_query(const std::string &db, const std::string &sql)
{
    DbRows_t rows;

    Sqlite3_prepare func_prepare = reinterpret_cast<Sqlite3_prepare>(g_WeChatWinDllAddr + SQLITE3_PREPARE_OFFSET);
    Sqlite3_step func_step       = reinterpret_cast<Sqlite3_step>(g_WeChatWinDllAddr + SQLITE3_STEP_OFFSET);
    Sqlite3_column_count func_column_count
        = reinterpret_cast<Sqlite3_column_count>(g_WeChatWinDllAddr + SQLITE3_COLUMN_COUNT_OFFSET);
    Sqlite3_column_name func_column_name
        = reinterpret_cast<Sqlite3_column_name>(g_WeChatWinDllAddr + SQLITE3_COLUMN_NAME_OFFSET);
    Sqlite3_column_type func_column_type
        = reinterpret_cast<Sqlite3_column_type>(g_WeChatWinDllAddr + SQLITE3_COLUMN_TYPE_OFFSET);
    Sqlite3_column_blob func_column_blob
        = reinterpret_cast<Sqlite3_column_blob>(g_WeChatWinDllAddr + SQLITE3_COLUMN_BLOB_OFFSET);
    Sqlite3_column_bytes func_column_bytes
        = reinterpret_cast<Sqlite3_column_bytes>(g_WeChatWinDllAddr + SQLITE3_COLUMN_BYTES_OFFSET);
    Sqlite3_finalize func_finalize = reinterpret_cast<Sqlite3_finalize>(g_WeChatWinDllAddr + SQLITE3_FINALIZE_OFFSET);

    if (db_map.empty()) {
        db_map = get_db_handles();
    }

    auto it = db_map.find(db);
    if (it == db_map.end() || it->second == 0) {
        LOG_WARN("Empty handle for database '{}', retrying...", db);
        db_map = get_db_handles();
        it     = db_map.find(db);
        if (it == db_map.end() || it->second == 0) {
            LOG_ERROR("Failed to get handle for database '{}'", db);
            return rows;
        }
    }

    QWORD *stmt;
    int rc = func_prepare(it->second, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("SQL prepare failed for '{}': error code {}", db, rc);
        return rows;
    }

    while (func_step(stmt) == SQLITE_ROW) {
        DbRow_t row;
        int col_count = func_column_count(stmt);
        for (int i = 0; i < col_count; i++) {
            DbField_t field;
            field.type   = func_column_type(stmt, i);
            field.column = func_column_name(stmt, i);

            int length       = func_column_bytes(stmt, i);
            const void *blob = func_column_blob(stmt, i);
            if (length > 0 && field.type != SQLITE_NULL) {
                field.content.resize(length);
                std::memcpy(field.content.data(), blob, length);
            }
            row.push_back(field);
        }
        rows.push_back(row);
    }

    func_finalize(stmt);
    return rows;
}

int get_local_id_and_dbidx(uint64_t id, uint64_t *local_id, uint32_t *db_idx)
{
    if (!local_id || !db_idx) {
        LOG_ERROR("Invalid pointer arguments!");
        return -1;
    }

    QWORD msg_mgr_addr = util::get_qword(g_WeChatWinDllAddr + OFFSET_DB_MSG_MGR);
    int db_index       = static_cast<int>(util::get_qword(msg_mgr_addr + 0x68)); // 总不能 int 还不够吧？
    QWORD p_start      = util::get_qword(msg_mgr_addr + 0x50);

    *db_idx = 0;
    for (int i = db_index - 1; i >= 0; i--) { // 从后往前遍历
        QWORD db_addr = util::get_qword(p_start + i * 0x08);
        if (!db_addr) {
            continue;
        }

        std::string dbname = util::w2s(util::get_pp_wstring(db_addr));
        db_map[dbname]     = util::get_qword(db_addr + 0x78);

        std::string sql = "SELECT localId FROM MSG WHERE MsgSvrID=" + std::to_string(id) + ";";
        DbRows_t rows   = exec_db_query(dbname, sql);

        if (rows.empty() || rows.front().empty()) {
            continue;
        }

        const DbField_t &field = rows.front().front();
        if (field.column != "localId" || field.type != SQLITE_INTEGER) {
            continue;
        }

        std::string id_str(field.content.begin(), field.content.end());
        try {
            *local_id = std::stoull(id_str);
        } catch (const std::exception &e) {
            LOG_ERROR("Failed to parse localId: {}", e.what());
            continue;
        }

        *db_idx = static_cast<uint32_t>(util::get_qword(util::get_qword(db_addr + 0x28) + 0x1E8) >> 32);
        return 0;
    }

    return -1;
}

std::vector<uint8_t> get_audio_data(uint64_t id)
{
    QWORD msg_mgr_addr = util::get_qword(g_WeChatWinDllAddr + OFFSET_DB_MSG_MGR);
    int db_index       = static_cast<int>(util::get_qword(msg_mgr_addr + 0x68));

    std::string sql = "SELECT Buf FROM Media WHERE Reserved0=" + std::to_string(id) + ";";
    for (int i = db_index - 1; i >= 0; i--) {
        std::string dbname = "MediaMSG" + std::to_string(i) + ".db";
        DbRows_t rows      = exec_db_query(dbname, sql);

        if (rows.empty() || rows.front().empty()) {
            continue;
        }

        const DbField_t &field = rows.front().front();
        if (field.column != "Buf" || field.content.empty()) {
            continue;
        }

        // 首字节为 0x02，估计是混淆用的？去掉。
        if (field.content.front() == 0x02) {
            return std::vector<uint8_t>(field.content.begin() + 1, field.content.end());
        }

        return field.content;
    }

    return {};
}

bool rpc_get_db_names(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_DB_NAMES>(out, len, [&](Response &rsp) {
        DbNames_t names                = get_db_names();
        rsp.msg.dbs.names.funcs.encode = encode_dbnames;
        rsp.msg.dbs.names.arg          = &names;
    });
}

bool rpc_get_db_tables(const std::string &db, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_DB_TABLES>(out, len, [&](Response &rsp) {
        DbTables_t tables                  = get_db_tables(db);
        rsp.msg.tables.tables.funcs.encode = encode_tables;
        rsp.msg.tables.tables.arg          = &tables;
    });
}

bool rpc_exec_db_query(const std::string &db, const std::string &sql, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_EXEC_DB_QUERY>(out, len, [&](Response &rsp) {
        DbRows_t rows                  = exec_db_query(db, sql);
        rsp.msg.rows.rows.funcs.encode = encode_rows;
        rsp.msg.rows.rows.arg          = &rows;
    });
}

} // namespace exec_sql
