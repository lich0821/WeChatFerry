#include "database_executor.h"

#include <algorithm>
#include <iterator>
#include <map>

#include "log.hpp"
#include "pb_util.h"
#include "rpc_helper.h"
#include "sqlite3.h"
#include "util.h"

#define OFFSET_DB_INSTANCE     0x2FFDDC8
#define OFFSET_DB_MICROMSG     0x68
#define OFFSET_DB_CHAT_MSG     0x1C0
#define OFFSET_DB_MISC         0x3D8
#define OFFSET_DB_EMOTION      0x558
#define OFFSET_DB_MEDIA        0x9B8
#define OFFSET_DB_BIZCHAT_MSG  0x1120
#define OFFSET_DB_FUNCTION_MSG 0x11B0
#define OFFSET_DB_NAME         0x14
#define OFFSET_DB_MSG_MGR      0x30403B8

extern uint32_t g_WeChatWinDllAddr;

namespace db
{

using db_map_t = std::map<std::string, DWORD>;

static db_map_t db_map;

static void get_db_handle(DWORD base, uint32_t offset)
{
    wchar_t *wsp = (wchar_t *)(*(DWORD *)(base + offset + OFFSET_DB_NAME));
    std::string dbname = util::w2s(std::wstring(wsp));
    db_map[dbname]     = util::get_dword(base + offset);
}

static void get_msg_db_handle(DWORD msg_mgr_addr)
{
    uint32_t db_index = util::get_dword(msg_mgr_addr + 0x38);
    uint32_t p_start  = util::get_dword(msg_mgr_addr + 0x2C);
    for (uint32_t i = 0; i < db_index; i++) {
        uint32_t db_addr = util::get_dword(p_start + i * 0x04);
        if (db_addr) {
            std::string dbname = util::w2s(util::get_wstring(db_addr));
            db_map[dbname]     = util::get_dword(db_addr + 0x60);

            uint32_t mmdb_addr  = util::get_dword(db_addr + 0x14);
            std::string mmdbname = util::w2s(util::get_wstring(mmdb_addr + 0x4C));
            db_map[mmdbname]     = util::get_dword(mmdb_addr + 0x38);
        }
    }
}

static db_map_t get_db_handles()
{
    db_map.clear();

    uint32_t db_instance_addr = util::get_dword(g_WeChatWinDllAddr + OFFSET_DB_INSTANCE);

    get_db_handle(db_instance_addr, OFFSET_DB_MICROMSG);
    get_db_handle(db_instance_addr, OFFSET_DB_CHAT_MSG);
    get_db_handle(db_instance_addr, OFFSET_DB_MISC);
    get_db_handle(db_instance_addr, OFFSET_DB_EMOTION);
    get_db_handle(db_instance_addr, OFFSET_DB_MEDIA);
    get_db_handle(db_instance_addr, OFFSET_DB_FUNCTION_MSG);

    get_msg_db_handle(util::get_dword(g_WeChatWinDllAddr + OFFSET_DB_MSG_MGR));

    return db_map;
}

DbNames_t get_db_names()
{
    DbNames_t names;
    if (db_map.empty()) {
        db_map = get_db_handles();
    }

    for (auto &[k, v] : db_map) {
        names.push_back(k);
    }

    return names;
}

static int cb_get_tables(void *ret, int argc, char **argv, char **azColName)
{
    DbTables_t *tbls = (DbTables_t *)ret;
    DbTable_t tbl;
    for (int i = 0; i < argc; i++) {
        if (strcmp(azColName[i], "name") == 0) {
            tbl.name = argv[i] ? argv[i] : "";
        } else if (strcmp(azColName[i], "sql") == 0) {
            std::string sql(argv[i]);
            sql.erase(std::remove(sql.begin(), sql.end(), '\t'), sql.end());
            tbl.sql = sql.c_str();
        }
    }
    tbls->push_back(tbl);
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

    const char *sql             = "select name, sql from sqlite_master where type=\"table\";";
    Sqlite3_exec p_sqlite3_exec = (Sqlite3_exec)(g_WeChatWinDllAddr + SQLITE3_EXEC_OFFSET);

    p_sqlite3_exec(it->second, sql, (Sqlite3_callback)cb_get_tables, (void *)&tables, 0);

    return tables;
}

DbRows_t exec_db_query(const std::string &db, const std::string &sql)
{
    DbRows_t rows;
    Sqlite3_prepare func_prepare           = (Sqlite3_prepare)(g_WeChatWinDllAddr + SQLITE3_PREPARE_OFFSET);
    Sqlite3_step func_step                 = (Sqlite3_step)(g_WeChatWinDllAddr + SQLITE3_STEP_OFFSET);
    Sqlite3_column_count func_column_count = (Sqlite3_column_count)(g_WeChatWinDllAddr + SQLITE3_COLUMN_COUNT_OFFSET);
    Sqlite3_column_name func_column_name   = (Sqlite3_column_name)(g_WeChatWinDllAddr + SQLITE3_COLUMN_NAME_OFFSET);
    Sqlite3_column_type func_column_type   = (Sqlite3_column_type)(g_WeChatWinDllAddr + SQLITE3_COLUMN_TYPE_OFFSET);
    Sqlite3_column_blob func_column_blob   = (Sqlite3_column_blob)(g_WeChatWinDllAddr + SQLITE3_COLUMN_BLOB_OFFSET);
    Sqlite3_column_bytes func_column_bytes = (Sqlite3_column_bytes)(g_WeChatWinDllAddr + SQLITE3_COLUMN_BYTES_OFFSET);

    if (db_map.empty()) {
        db_map = get_db_handles();
    }

    DWORD *stmt;
    int rc = func_prepare(db_map[db], sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
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
            if (length && (field.type != 5)) {
                field.content.reserve(length);
                copy((uint8_t *)blob, (uint8_t *)blob + length, back_inserter(field.content));
            }
            row.push_back(field);
        }
        rows.push_back(row);
    }
    return rows;
}

int get_local_id_and_dbidx(uint64_t id, uint64_t *local_id, uint32_t *db_idx)
{
    uint32_t msg_mgr_addr = util::get_dword(g_WeChatWinDllAddr + OFFSET_DB_MSG_MGR);
    uint32_t db_index     = util::get_dword(msg_mgr_addr + 0x38);
    uint32_t p_start      = util::get_dword(msg_mgr_addr + 0x2C);

    *db_idx = 0;
    for (int i = db_index - 1; i >= 0; i--) {
        uint32_t db_addr = util::get_dword(p_start + i * 0x04);
        if (db_addr) {
            std::string dbname = util::w2s(util::get_wstring(db_addr));
            db_map[dbname]     = util::get_dword(db_addr + 0x60);
            std::string sql    = "SELECT localId FROM MSG WHERE MsgSvrID=" + to_string(id) + ";";
            DbRows_t rows      = exec_db_query(dbname, sql);
            if (rows.empty()) {
                continue;
            }
            DbRow_t row = rows.front();
            if (row.empty()) {
                continue;
            }
            DbField_t field = row.front();
            if ((field.column.compare("localId") != 0) && (field.type != 1)) {
                continue;
            }

            *local_id = strtoull((const char *)(field.content.data()), NULL, 10);
            *db_idx   = util::get_dword(util::get_dword(db_addr + 0x18) + 0x144);

            return 0;
        }
    }

    return -1;
}

std::vector<uint8_t> get_audio_data(uint64_t msg_id)
{
    uint32_t msg_mgr_addr = util::get_dword(g_WeChatWinDllAddr + OFFSET_DB_MSG_MGR);
    uint32_t db_index     = util::get_dword(msg_mgr_addr + 0x38);

    std::string sql = "SELECT Buf from Media  WHERE Reserved0=" + to_string(msg_id) + ";";
    for (int i = db_index - 1; i >= 0; i--) {
        std::string dbname = "MediaMSG" + to_string(i) + ".db";
        DbRows_t rows      = exec_db_query(dbname, sql);
        if (rows.empty()) {
            continue;
        }
        DbRow_t row = rows.front();
        if (row.empty()) {
            continue;
        }
        DbField_t field = row.front();
        if (field.column.compare("Buf") != 0) {
            continue;
        }

        std::vector<uint8_t> rv(field.content.begin() + 1, field.content.end());
        return rv;
    }

    return std::vector<uint8_t>();
}

bool rpc_get_db_names(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_DB_NAMES>(out, len, [](Response &rsp) {
        DbNames_t dbnames              = get_db_names();
        rsp.msg.dbs.names.funcs.encode = encode_dbnames;
        rsp.msg.dbs.names.arg          = &dbnames;
    });
}

bool rpc_get_db_tables(const std::string &db, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_DB_TABLES>(out, len, [&db](Response &rsp) {
        DbTables_t tables                  = get_db_tables(db);
        rsp.msg.tables.tables.funcs.encode = encode_tables;
        rsp.msg.tables.tables.arg          = &tables;
    });
}

bool rpc_exec_db_query(const DbQuery &query, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_EXEC_DB_QUERY>(out, len, [&query](Response &rsp) {
        if ((query.db == NULL) || (query.sql == NULL)) {
            LOG_ERROR("Empty db or sql.");
            DbRows_t rows;
            rsp.msg.rows.rows.funcs.encode = encode_rows;
            rsp.msg.rows.rows.arg          = &rows;
        } else {
            std::string db(query.db);
            std::string sql(query.sql);
            DbRows_t rows                  = exec_db_query(db, sql);
            rsp.msg.rows.rows.funcs.encode = encode_rows;
            rsp.msg.rows.rows.arg          = &rows;
        }
    });
}

} // namespace db
