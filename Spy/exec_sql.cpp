#include <map>
#include <string>

#include "exec_sql.h"
#include "load_calls.h"
#include "util.h"

using namespace std;

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

typedef map<wstring, DWORD> dbMap_t;
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

static int cbGetTables(void *ret, int argc, char **argv, char **azColName)
{
    vector<RpcTables_t> *p = (vector<RpcTables_t> *)ret;
    RpcTables_t tbl        = { 0 };
    for (int i = 0; i < argc; i++) {
        if (strcmp(azColName[i], "name") == 0) {
            tbl.table = argv[i] ? GetBstrFromString(argv[i]) : NULL;
        } else if (strcmp(azColName[i], "sql") == 0) {
            tbl.sql = argv[i] ? GetBstrFromString(argv[i]) : NULL;
        }
    }
    p->push_back(tbl);
    return 0;
}

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
        wstring dbName = wstring((wchar_t *)(*(DWORD *)(dwHandle + g_WxCalls.sql.name)));
        DWORD handle   = *(DWORD *)(dwHandle + g_WxCalls.sql.slot);
        if (handle) {
            dbMap[dbName] = handle;
        }

        sqlHandleBeginAddr += 0x04;
    }
    return dbMap;
}

vector<wstring> GetDbNames()
{
    vector<wstring> vDbs;
    if (dbMap.empty()) {
        dbMap = GetDbHandles();
    }
    for (auto it = dbMap.begin(); it != dbMap.end(); it++) {
        vDbs.push_back(it->first);
    }
    return vDbs;
}

vector<RpcTables_t> GetDbTables(wstring db)
{
    vector<RpcTables_t> vTables;
    const char *sql             = "select * from sqlite_master where type=\"table\";";
    Sqlite3_exec p_Sqlite3_exec = (Sqlite3_exec)(g_WeChatWinDllAddr + g_WxCalls.sql.exec);

    if (dbMap.empty()) {
        dbMap = GetDbHandles();
    }

    auto it = dbMap.find(db);
    if (it != dbMap.end()) {
        p_Sqlite3_exec(it->second, sql, (sqlite3_callback)cbGetTables, &vTables, 0);
    }

    return vTables;
}

vector<vector<RpcSqlResult_t>> ExecDbQuery(wstring db, wstring sql)
{
    vector<vector<RpcSqlResult_t>> vvSqlResult;

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
    int rc = func_prepare(dbMap[db], Wstring2String(sql).c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        return vvSqlResult;
    }

    wchar_t buffer[128] = { 0 };
    while (func_step(stmt) == SQLITE_ROW) {
        vector<RpcSqlResult_t> vResult;
        int col_count = func_column_count(stmt);
        for (int i = 0; i < col_count; i++) {
            RpcSqlResult_t result = { 0 };
            result.type           = func_column_type(stmt, i);
            result.column         = GetBstrFromString(func_column_name(stmt, i));
            int length            = func_column_bytes(stmt, i);
            const void *blob      = func_column_blob(stmt, i);
            if (length && (result.type != 5)) {
                result.content = GetBstrFromByteArray((byte *)blob, length);
            }

            vResult.push_back(result);
        }
        vvSqlResult.push_back(vResult);
    }

    return vvSqlResult;
}
