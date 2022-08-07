#include <map>
#include <string>

#include "exec_sql.h"
#include "load_calls.h"
#include "util.h"

using namespace std;

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

vector<wstring> GetDbNames()
{
    vector<wstring> vDbs;
    if (dbMap.empty()) {
        DWORD sqlHandleBaseAddr  = *(DWORD *)(g_WeChatWinDllAddr + g_WxCalls.sql.base);
        DWORD sqlHandleBeginAddr = *(DWORD *)(sqlHandleBaseAddr + g_WxCalls.sql.start);
        DWORD sqlHandleEndAddr   = *(DWORD *)(sqlHandleBaseAddr + g_WxCalls.sql.end);
        while (sqlHandleBeginAddr < sqlHandleEndAddr) {
            DWORD dwHandle = *(DWORD *)sqlHandleBeginAddr;
            dbMap[wstring((wchar_t *)(*(DWORD *)(dwHandle + g_WxCalls.sql.name)))]
                = *(DWORD *)(dwHandle + g_WxCalls.sql.slot);
            sqlHandleBeginAddr += 0x04;
        }
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

    auto it = dbMap.find(db);
    if (it != dbMap.end()) {
        p_Sqlite3_exec(it->second, sql, (sqlite3_callback)cbGetTables, &vTables, 0);
    }

    return vTables;
}
