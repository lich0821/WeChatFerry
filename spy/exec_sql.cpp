#include <iterator>

#include "exec_sql.h"
#include "load_calls.h"
#include "util.h"

#define SQLITE_OK         0   /* Successful result */
#define SQLITE_ERROR      1   /* Generic error */
#define SQLITE_INTERNAL   2   /* Internal logic error in SQLite */
#define SQLITE_PERM       3   /* Access permission denied */
#define SQLITE_ABORT      4   /* Callback routine requested an abort */
#define SQLITE_BUSY       5   /* The database file is locked */
#define SQLITE_LOCKED     6   /* A table in the database is locked */
#define SQLITE_NOMEM      7   /* A malloc() failed */
#define SQLITE_READONLY   8   /* Attempt to write a readonly database */
#define SQLITE_INTERRUPT  9   /* Operation terminated by sqlite3_interrupt()*/
#define SQLITE_IOERR      10  /* Some kind of disk I/O error occurred */
#define SQLITE_CORRUPT    11  /* The database disk image is malformed */
#define SQLITE_NOTFOUND   12  /* Unknown opcode in sqlite3_file_control() */
#define SQLITE_FULL       13  /* Insertion failed because database is full */
#define SQLITE_CANTOPEN   14  /* Unable to open the database file */
#define SQLITE_PROTOCOL   15  /* Database lock protocol error */
#define SQLITE_EMPTY      16  /* Internal use only */
#define SQLITE_SCHEMA     17  /* The database schema changed */
#define SQLITE_TOOBIG     18  /* String or BLOB exceeds size limit */
#define SQLITE_CONSTRAINT 19  /* Abort due to constraint violation */
#define SQLITE_MISMATCH   20  /* Data type mismatch */
#define SQLITE_MISUSE     21  /* Library used incorrectly */
#define SQLITE_NOLFS      22  /* Uses OS features not supported on host */
#define SQLITE_AUTH       23  /* Authorization denied */
#define SQLITE_FORMAT     24  /* Not used */
#define SQLITE_RANGE      25  /* 2nd parameter to sqlite3_bind out of range */
#define SQLITE_NOTADB     26  /* File opened that is not a database file */
#define SQLITE_NOTICE     27  /* Notifications from sqlite3_log() */
#define SQLITE_WARNING    28  /* Warnings from sqlite3_log() */
#define SQLITE_ROW        100 /* sqlite3_step() has another row ready */
#define SQLITE_DONE       101 /* sqlite3_step() has finished executing */

#define SQLITE_INTEGER 1
#define SQLITE_FLOAT   2
#define SQLITE_TEXT    3
#define SQLITE_BLOB    4
#define SQLITE_NULL    5

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

dbMap_t GetDbHandles()
{
    if (!dbMap.empty())
        return dbMap;

    g_WeChatWinDllAddr       = (DWORD)GetModuleHandle(L"WeChatWin.dll");
    DWORD sqlHandleBaseAddr  = *(DWORD *)(g_WeChatWinDllAddr + g_WxCalls.sql.base);
    DWORD sqlHandleBeginAddr = *(DWORD *)(sqlHandleBaseAddr + g_WxCalls.sql.start);
    DWORD sqlHandleEndAddr   = *(DWORD *)(sqlHandleBaseAddr + g_WxCalls.sql.end);
    while (sqlHandleBeginAddr < sqlHandleEndAddr) {
        DWORD dwHandle = *(DWORD *)sqlHandleBeginAddr;
        string dbName  = Wstring2String(wstring((wchar_t *)(*(DWORD *)(dwHandle + g_WxCalls.sql.name))));
        DWORD handle   = *(DWORD *)(dwHandle + g_WxCalls.sql.slot);
        if (handle) {
            dbMap[dbName] = handle;
        }

        sqlHandleBeginAddr += 0x04;
    }
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
