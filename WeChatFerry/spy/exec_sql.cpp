#include <iterator>

#include "exec_sql.h"
#include "load_calls.h"
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

extern DWORD g_WeChatWinDllAddr;

typedef map<string, DWORD> dbMap_t;
static dbMap_t dbMap;

static void GetDbHandle(DWORD base, DWORD offset)
{
    wchar_t *wsp;
    wsp           = (wchar_t *)(*(DWORD *)(base + offset + OFFSET_DB_NAME));
    string dbname = Wstring2String(wstring(wsp));
    dbMap[dbname] = *(DWORD *)(base + offset);
}

dbMap_t GetDbHandles()
{
    dbMap.clear();

    DWORD dbInstanceAddr = *(DWORD *)(g_WeChatWinDllAddr + OFFSET_DB_INSTANCE);

    GetDbHandle(dbInstanceAddr, OFFSET_DB_MICROMSG);     // MicroMsg.db
    GetDbHandle(dbInstanceAddr, OFFSET_DB_CHAT_MSG);     // ChatMsg.db
    GetDbHandle(dbInstanceAddr, OFFSET_DB_MISC);         // Misc.db
    GetDbHandle(dbInstanceAddr, OFFSET_DB_EMOTION);      // Emotion.db
    GetDbHandle(dbInstanceAddr, OFFSET_DB_MEDIA);        // Media.db
    GetDbHandle(dbInstanceAddr, OFFSET_DB_FUNCTION_MSG); // Function.db

    return dbMap;
}

DbNames_t GetDbNames()
{
    DbNames_t names;
    if (dbMap.empty()) {
        dbMap = GetDbHandles();
    }

    for (auto &[k, v] : dbMap) {
        names.push_back(k);
    }

    return names;
}

static int cbGetTables(void *ret, int argc, char **argv, char **azColName)
{
    DbTables_t *tbls = (DbTables_t *)ret;
    DbTable_t tbl;
    for (int i = 0; i < argc; i++) {
        if (strcmp(azColName[i], "name") == 0) {
            tbl.name = argv[i] ? argv[i] : "";
        } else if (strcmp(azColName[i], "sql") == 0) {
            string sql(argv[i]);
            sql.erase(std::remove(sql.begin(), sql.end(), '\t'), sql.end());
            tbl.sql = sql.c_str();
        }
    }
    tbls->push_back(tbl);
    return 0;
}

DbTables_t GetDbTables(const string db)
{
    DbTables_t tables;
    if (dbMap.empty()) {
        dbMap = GetDbHandles();
    }

    auto it = dbMap.find(db);
    if (it == dbMap.end()) {
        return tables; // DB not found
    }

    const char *sql             = "select name, sql from sqlite_master where type=\"table\";";
    Sqlite3_exec p_Sqlite3_exec = (Sqlite3_exec)(g_WeChatWinDllAddr + SQLITE3_EXEC_OFFSET);

    p_Sqlite3_exec(it->second, sql, (Sqlite3_callback)cbGetTables, (void *)&tables, 0);

    return tables;
}

DbRows_t ExecDbQuery(const string db, const string sql)
{
    DbRows_t rows;
    Sqlite3_prepare func_prepare           = (Sqlite3_prepare)(g_WeChatWinDllAddr + SQLITE3_PREPARE_OFFSET);
    Sqlite3_step func_step                 = (Sqlite3_step)(g_WeChatWinDllAddr + SQLITE3_STEP_OFFSET);
    Sqlite3_column_count func_column_count = (Sqlite3_column_count)(g_WeChatWinDllAddr + SQLITE3_COLUMN_COUNT_OFFSET);
    Sqlite3_column_name func_column_name   = (Sqlite3_column_name)(g_WeChatWinDllAddr + SQLITE3_COLUMN_NAME_OFFSET);
    Sqlite3_column_type func_column_type   = (Sqlite3_column_type)(g_WeChatWinDllAddr + SQLITE3_COLUMN_TYPE_OFFSET);
    Sqlite3_column_blob func_column_blob   = (Sqlite3_column_blob)(g_WeChatWinDllAddr + SQLITE3_COLUMN_BLOB_OFFSET);
    Sqlite3_column_bytes func_column_bytes = (Sqlite3_column_bytes)(g_WeChatWinDllAddr + SQLITE3_COLUMN_BYTES_OFFSET);
    Sqlite3_finalize func_finalize         = (Sqlite3_finalize)(g_WeChatWinDllAddr + SQLITE3_FINALIZE_OFFSET);

    if (dbMap.empty()) {
        dbMap = GetDbHandles();
    }

    DWORD *stmt;
    int rc = func_prepare(dbMap[db], sql.c_str(), -1, &stmt, 0);
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
