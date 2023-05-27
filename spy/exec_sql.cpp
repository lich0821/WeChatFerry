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

extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

typedef map<string, DWORD> dbMap_t;
static dbMap_t dbMap;

// 回调函数指针
typedef int (*sqlite3_callback)(void *, int, char **, char **);

// sqlite3_exec函数指针
typedef int(__cdecl *Sqlite3_exec)(DWORD,            /* The database on which the SQL executes */
                                   const char *,     /* The SQL to be executed */
                                   sqlite3_callback, /* Invoke this callback routine */
                                   void *,           /* First argument to xCallback() */
                                   char **           /* Write error messages here */
);
typedef int(__cdecl *Sqlite3_prepare)(DWORD, const char *, int, DWORD **, int);
typedef int(__cdecl *Sqlite3_step)(DWORD *);
typedef int(__cdecl *Sqlite3_column_count)(DWORD *);
typedef const char *(__cdecl *Sqlite3_column_name)(DWORD *, int);
typedef int(__cdecl *Sqlite3_column_type)(DWORD *, int);
typedef const void *(__cdecl *Sqlite3_column_blob)(DWORD *, int);
typedef int(__cdecl *Sqlite3_column_bytes)(DWORD *, int);
typedef int(__cdecl *Sqlite3_finalize)(DWORD *);

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
    Sqlite3_exec p_Sqlite3_exec = (Sqlite3_exec)(g_WeChatWinDllAddr + g_WxCalls.sql.exec);

    p_Sqlite3_exec(it->second, sql, (sqlite3_callback)cbGetTables, (void *)&tables, 0);

    return tables;
}

DbRows_t ExecDbQuery(const string db, const string sql)
{
    DbRows_t rows;
    Sqlite3_prepare func_prepare           = (Sqlite3_prepare)(g_WeChatWinDllAddr + 0x14227F0);
    Sqlite3_step func_step                 = (Sqlite3_step)(g_WeChatWinDllAddr + 0x13EA780);
    Sqlite3_column_count func_column_count = (Sqlite3_column_count)(g_WeChatWinDllAddr + 0x13EACD0);
    Sqlite3_column_name func_column_name   = (Sqlite3_column_name)(g_WeChatWinDllAddr + 0x13EB630);
    Sqlite3_column_type func_column_type   = (Sqlite3_column_type)(g_WeChatWinDllAddr + 0x13EB470);
    Sqlite3_column_blob func_column_blob   = (Sqlite3_column_blob)(g_WeChatWinDllAddr + 0x13EAD10);
    Sqlite3_column_bytes func_column_bytes = (Sqlite3_column_bytes)(g_WeChatWinDllAddr + 0x13EADD0);
    Sqlite3_finalize func_finalize         = (Sqlite3_finalize)(g_WeChatWinDllAddr + 0x13E9730);

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
