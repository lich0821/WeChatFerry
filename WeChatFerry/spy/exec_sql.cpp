#include <iterator>

#include "exec_sql.h"
#include "log.h"
#include "sqlite3.h"
#include "util.h"

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

extern UINT64 g_WeChatWinDllAddr;

typedef map<string, QWORD> dbMap_t;
static dbMap_t dbMap;

static void GetDbHandle(QWORD base, QWORD offset)
{
    wchar_t *wsp  = (wchar_t *)(*(QWORD *)(base + offset + OFFSET_DB_NAME));
    string dbname = Wstring2String(wstring(wsp));
    dbMap[dbname] = GET_QWORD(base + offset);
}

static void GetMsgDbHandle(QWORD msgMgrAddr)
{
    QWORD dbIndex = GET_QWORD(msgMgrAddr + 0x68);
    QWORD pStart  = GET_QWORD(msgMgrAddr + 0x50);
    for (uint32_t i = 0; i < dbIndex; i++) {
        QWORD dbAddr = GET_QWORD(pStart + i * 0x08);
        if (dbAddr) {
            // MSGi.db
            string dbname = Wstring2String(GET_WSTRING(dbAddr));
            dbMap[dbname] = GET_QWORD(dbAddr + 0x78);

            // MediaMsgi.db
            QWORD mmdbAddr  = GET_QWORD(dbAddr + 0x20);
            string mmdbname = Wstring2String(GET_WSTRING(mmdbAddr + 0x78));
            dbMap[mmdbname] = GET_QWORD(mmdbAddr + 0x50);
        }
    }
}

dbMap_t GetDbHandles()
{
    dbMap.clear();

    QWORD dbInstanceAddr = GET_QWORD(g_WeChatWinDllAddr + OFFSET_DB_INSTANCE);

    GetDbHandle(dbInstanceAddr, OFFSET_DB_MICROMSG);     // MicroMsg.db
    GetDbHandle(dbInstanceAddr, OFFSET_DB_CHAT_MSG);     // ChatMsg.db
    GetDbHandle(dbInstanceAddr, OFFSET_DB_MISC);         // Misc.db
    GetDbHandle(dbInstanceAddr, OFFSET_DB_EMOTION);      // Emotion.db
    GetDbHandle(dbInstanceAddr, OFFSET_DB_MEDIA);        // Media.db
    GetDbHandle(dbInstanceAddr, OFFSET_DB_FUNCTION_MSG); // Function.db

    GetMsgDbHandle(GET_QWORD(g_WeChatWinDllAddr + OFFSET_DB_MSG_MGR)); // MSGi.db & MediaMsgi.db

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

    QWORD *stmt;
    QWORD handle = dbMap[db];
    if (handle == 0) {
        LOG_WARN("Empty handle, retrying...");
        dbMap = GetDbHandles();
    }

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

int GetLocalIdandDbidx(uint64_t id, uint64_t *localId, uint32_t *dbIdx)
{
    QWORD msgMgrAddr = GET_QWORD(g_WeChatWinDllAddr + OFFSET_DB_MSG_MGR);
    int dbIndex      = (int)GET_QWORD(msgMgrAddr + 0x68); // 总不能 int 还不够吧？
    QWORD pStart     = GET_QWORD(msgMgrAddr + 0x50);

    *dbIdx = 0;
    for (int i = dbIndex - 1; i >= 0; i--) { // 从后往前遍历
        QWORD dbAddr = GET_QWORD(pStart + i * 0x08);
        if (dbAddr) {
            string dbname = Wstring2String(GET_WSTRING(dbAddr));
            dbMap[dbname] = GET_QWORD(dbAddr + 0x78);
            string sql    = "SELECT localId FROM MSG WHERE MsgSvrID=" + to_string(id) + ";";
            DbRows_t rows = ExecDbQuery(dbname, sql);
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

            *localId = strtoull((const char *)(field.content.data()), NULL, 10);
            *dbIdx   = (uint32_t)(GET_QWORD(GET_QWORD(dbAddr + 0x28) + 0x1E8) >> 32);

            return 0;
        }
    }

    return -1;
}

vector<uint8_t> GetAudioData(uint64_t id)
{
    QWORD msgMgrAddr = GET_QWORD(g_WeChatWinDllAddr + OFFSET_DB_MSG_MGR);
    int dbIndex      = (int)GET_QWORD(msgMgrAddr + 0x68);

    string sql = "SELECT Buf FROM Media WHERE Reserved0=" + to_string(id) + ";";
    for (int i = dbIndex - 1; i >= 0; i--) {
        string dbname = "MediaMSG" + to_string(i) + ".db";
        DbRows_t rows = ExecDbQuery(dbname, sql);
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

        // 首字节为 0x02，估计是混淆用的？去掉。
        vector<uint8_t> rv(field.content.begin() + 1, field.content.end());

        return rv;
    }

    return vector<uint8_t>();
}
