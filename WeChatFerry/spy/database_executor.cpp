#include "database_executor.h"

#include <algorithm>
#include <iterator>

#include "log.hpp"
#include "offsets.h"
#include "pb_util.h"
#include "rpc_helper.h"
#include "spy.h"
#include "sqlite3.h"
#include "util.h"

namespace db
{
namespace OsDb = Offsets::Db;

using db_map_t = std::map<std::string, QWORD>;
static db_map_t db_map;

static void get_db_handle(QWORD base, QWORD offset)
{
    auto *wsp          = reinterpret_cast<wchar_t *>(*(QWORD *)(base + offset + OsDb::NAME));
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
    QWORD db_instance_addr = util::get_qword(Spy::WeChatDll.load() + OsDb::INSTANCE);

    get_db_handle(db_instance_addr, OsDb::MICROMSG);     // MicroMsg.db
    get_db_handle(db_instance_addr, OsDb::CHAT_MSG);     // ChatMsg.db
    get_db_handle(db_instance_addr, OsDb::MISC);         // Misc.db
    get_db_handle(db_instance_addr, OsDb::EMOTION);      // Emotion.db
    get_db_handle(db_instance_addr, OsDb::MEDIA);        // Media.db
    get_db_handle(db_instance_addr, OsDb::FUNCTION_MSG); // Function.db

    get_msg_db_handle(util::get_qword(Spy::WeChatDll.load() + OsDb::MSG_I)); // MSGi.db & MediaMsgi.db
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

    constexpr const char *sql = "SELECT name FROM sqlite_master WHERE type='table';";
    auto p_sqlite3_exec       = Spy::getFunction<Sqlite3_exec>(OsDb::EXEC);
    p_sqlite3_exec(it->second, sql, (Sqlite3_callback)cb_get_tables, (void *)&tables, nullptr);

    return tables;
}

DbRows_t exec_db_query(const std::string &db, const std::string &sql)
{
    DbRows_t rows;

    auto func_prepare      = Spy::getFunction<Sqlite3_prepare>(OsDb::PREPARE);
    auto func_step         = Spy::getFunction<Sqlite3_step>(OsDb::STEP);
    auto func_column_count = Spy::getFunction<Sqlite3_column_count>(OsDb::COLUMN_COUNT);
    auto func_column_name  = Spy::getFunction<Sqlite3_column_name>(OsDb::COLUMN_NAME);
    auto func_column_type  = Spy::getFunction<Sqlite3_column_type>(OsDb::COLUMN_TYPE);
    auto func_column_blob  = Spy::getFunction<Sqlite3_column_blob>(OsDb::COLUMN_BLOB);
    auto func_column_bytes = Spy::getFunction<Sqlite3_column_bytes>(OsDb::COLUMN_BYTES);
    auto func_finalize     = Spy::getFunction<Sqlite3_finalize>(OsDb::FINALIZE);

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

    QWORD msg_mgr_addr = util::get_qword(Spy::WeChatDll.load() + OsDb::MSG_I);
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
    QWORD msg_mgr_addr = util::get_qword(Spy::WeChatDll.load() + OsDb::MSG_I);
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
    DbNames_t names = get_db_names();
    return fill_response<Functions_FUNC_GET_DB_NAMES>(out, len, [&](Response &rsp) {
        rsp.msg.dbs.names.funcs.encode = encode_dbnames;
        rsp.msg.dbs.names.arg          = &names;
    });
}

bool rpc_get_db_tables(const std::string &db, uint8_t *out, size_t *len)
{
    DbTables_t tables = get_db_tables(db);
    return fill_response<Functions_FUNC_GET_DB_TABLES>(out, len, [&](Response &rsp) {
        rsp.msg.tables.tables.funcs.encode = encode_tables;
        rsp.msg.tables.tables.arg          = &tables;
    });
}

bool rpc_exec_db_query(const DbQuery query, uint8_t *out, size_t *len)
{
    const std::string db(query.db);
    const std::string sql(query.sql);
    DbRows_t rows = exec_db_query(db, sql);
    return fill_response<Functions_FUNC_EXEC_DB_QUERY>(out, len, [&](Response &rsp) {
        rsp.msg.rows.rows.funcs.encode = encode_rows;
        rsp.msg.rows.rows.arg          = &rows;
    });
}

} // namespace db
