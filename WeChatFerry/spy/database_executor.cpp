#include "database_executor.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "log.hpp"
#include "offsets.h"
#include "pb_util.h"
#include "rpc_helper.h"
#include "util.h"

extern uint32_t g_WeChatWinDllAddr;

namespace db
{

namespace OsDb = Offsets::Database;

namespace
{

// SQLite 返回码与列类型
constexpr int SQLITE_OK      = 0;
constexpr int SQLITE_ROW     = 100;
constexpr int SQLITE_DONE    = 101;
constexpr int SQLITE_INTEGER = 1;
constexpr int SQLITE_FLOAT   = 2;
constexpr int SQLITE_TEXT    = 3;
constexpr int SQLITE_BLOB    = 4;
constexpr int SQLITE_NULL    = 5;

// WeChatWin.dll 进程内（已解密）SQLCipher/sqlite3 内部 API —— 均为 __cdecl。
// 地址来自 offsets.h::Database（RVA + g_WeChatWinDllAddr）。
using sqlite3_prepare_v2_fn   = int(__cdecl *)(void *, const char *, int, void **, const char **);
using sqlite3_step_fn         = int(__cdecl *)(void *);
using sqlite3_finalize_fn     = int(__cdecl *)(void *);
using sqlite3_column_count_fn = int(__cdecl *)(void *);
using sqlite3_column_name_fn  = const char *(__cdecl *)(void *, int);
using sqlite3_column_type_fn  = int(__cdecl *)(void *, int);
using sqlite3_column_blob_fn  = const void *(__cdecl *)(void *, int);
using sqlite3_column_bytes_fn = int(__cdecl *)(void *, int);
using sqlite3_column_text_fn  = const unsigned char *(__cdecl *)(void *, int);

struct SqliteApi {
    sqlite3_prepare_v2_fn prepare_v2;
    sqlite3_step_fn step;
    sqlite3_finalize_fn finalize;
    sqlite3_column_count_fn column_count;
    sqlite3_column_name_fn column_name;
    sqlite3_column_type_fn column_type;
    sqlite3_column_blob_fn column_blob;
    sqlite3_column_bytes_fn column_bytes;
    sqlite3_column_text_fn column_text;
};

const SqliteApi &sqlite_api()
{
    // g_WeChatWinDllAddr 在 InitSpy 期间即已就绪，RPC 线程首次调用时才构造，安全。
    static const SqliteApi api = {
        reinterpret_cast<sqlite3_prepare_v2_fn>(g_WeChatWinDllAddr + OsDb::PREPARE_V2),
        reinterpret_cast<sqlite3_step_fn>(g_WeChatWinDllAddr + OsDb::STEP),
        reinterpret_cast<sqlite3_finalize_fn>(g_WeChatWinDllAddr + OsDb::FINALIZE),
        reinterpret_cast<sqlite3_column_count_fn>(g_WeChatWinDllAddr + OsDb::COLUMN_COUNT),
        reinterpret_cast<sqlite3_column_name_fn>(g_WeChatWinDllAddr + OsDb::COLUMN_NAME),
        reinterpret_cast<sqlite3_column_type_fn>(g_WeChatWinDllAddr + OsDb::COLUMN_TYPE),
        reinterpret_cast<sqlite3_column_blob_fn>(g_WeChatWinDllAddr + OsDb::COLUMN_BLOB),
        reinterpret_cast<sqlite3_column_bytes_fn>(g_WeChatWinDllAddr + OsDb::COLUMN_BYTES),
        reinterpret_cast<sqlite3_column_text_fn>(g_WeChatWinDllAddr + OsDb::COLUMN_TEXT),
    };
    return api;
}

// 库名 → 已解密 sqlite3* 句柄
using db_map_t = std::map<std::string, uint32_t>;
db_map_t db_map;

// 读取 storage 对象内 storage+NAME 处的 std::wstring（MSVC 布局：_Bx 起始，容量位 +0x14；
// wchar_t 的 SSO 阈值为 8：容量 >= 8 走堆指针，否则内联缓冲）并取文件名（basename）。
std::string read_db_name(uint32_t wstr_addr)
{
    uint32_t capacity     = util::get_dword(wstr_addr + 0x14);
    const wchar_t *buffer = (capacity >= 8) ? reinterpret_cast<const wchar_t *>(util::get_dword(wstr_addr))
                                            : reinterpret_cast<const wchar_t *>(wstr_addr);
    if ((buffer == nullptr) || (*buffer == L'\0')) {
        return "";
    }

    std::wstring path(buffer);
    size_t pos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        path = path.substr(pos + 1);
    }
    return util::w2s(path);
}

// 遍历 AccountStorageMgr 主 storage 数组，建立 库名 → sqlite3* 映射。
void refresh_db_map()
{
    db_map.clear();

    uint32_t base = g_WeChatWinDllAddr;
    if (base == 0) {
        return;
    }

    uint32_t mgr = util::get_dword(base + OsDb::INSTANCE);
    if (mgr == 0) {
        LOG_ERROR("AccountStorageMgr instance is null.");
        return;
    }

    uint32_t begin = util::get_dword(mgr + OsDb::START);
    uint32_t end   = util::get_dword(mgr + OsDb::END);
    if ((begin == 0) || (end < begin)) {
        return;
    }

    for (uint32_t p = begin; p < end; p += 4) {
        uint32_t storage = util::get_dword(p);
        if (storage == 0) {
            continue;
        }

        uint32_t handle = util::get_dword(storage + OsDb::SLOT);
        if (handle == 0) {
            continue;
        }

        std::string name = read_db_name(storage + OsDb::NAME);
        if (name.empty()) {
            continue;
        }

        db_map.emplace(name, handle);
    }
}

uint32_t find_db_handle(const std::string &db)
{
    if (db_map.empty()) {
        refresh_db_map();
    }

    auto it = db_map.find(db);
    if (it != db_map.end() && it->second != 0) {
        return it->second;
    }

    // 句柄可能因重登录等变化，重建一次再试。
    refresh_db_map();
    it = db_map.find(db);
    return (it == db_map.end()) ? 0 : it->second;
}

std::string field_to_string(const DbField_t &field)
{
    return std::string(field.content.begin(), field.content.end());
}

int parse_db_index(const std::string &dbname, const char *prefix)
{
    const size_t prefix_len = strlen(prefix);
    if ((dbname.size() <= prefix_len + 3) || (dbname.compare(0, prefix_len, prefix) != 0) ||
        (dbname.compare(dbname.size() - 3, 3, ".db") != 0)) {
        return -1;
    }

    std::string idx = dbname.substr(prefix_len, dbname.size() - prefix_len - 3);
    if (idx.empty() || !std::all_of(idx.begin(), idx.end(), [](char ch) { return ch >= '0' && ch <= '9'; })) {
        return -1;
    }

    return atoi(idx.c_str());
}

std::vector<std::pair<int, std::string>> find_dbs_by_prefix(const char *prefix)
{
    if (db_map.empty()) {
        refresh_db_map();
    }

    std::vector<std::pair<int, std::string>> result;
    for (const auto &[name, handle] : db_map) {
        (void)handle;
        int idx = parse_db_index(name, prefix);
        if (idx >= 0) {
            result.emplace_back(idx, name);
        }
    }

    std::sort(result.begin(), result.end(), [](const auto &lhs, const auto &rhs) { return lhs.first > rhs.first; });
    return result;
}

} // namespace

DbNames_t get_db_names()
{
    if (db_map.empty()) {
        refresh_db_map();
    }

    DbNames_t names;
    names.reserve(db_map.size());
    for (const auto &[name, handle] : db_map) {
        (void)handle;
        names.push_back(name);
    }
    return names;
}

DbTables_t get_db_tables(const std::string &db)
{
    DbTables_t tables;

    DbRows_t rows = exec_db_query(db, "SELECT name, sql FROM sqlite_master WHERE type = 'table' ORDER BY name;");
    for (const auto &row : rows) {
        DbTable_t table;
        for (const auto &field : row) {
            if (field.column == "name") {
                table.name = field_to_string(field);
            } else if (field.column == "sql") {
                std::string sql = field_to_string(field);
                sql.erase(std::remove(sql.begin(), sql.end(), '\t'), sql.end());
                table.sql = sql;
            }
        }
        tables.push_back(table);
    }
    return tables;
}

DbRows_t exec_db_query(const std::string &db, const std::string &sql)
{
    DbRows_t rows;

    uint32_t handle = find_db_handle(db);
    if (handle == 0) {
        LOG_ERROR("Failed to get handle for database '{}'.", db);
        return rows;
    }

    const SqliteApi &api = sqlite_api();

    void *stmt = nullptr;
    int rc     = api.prepare_v2(reinterpret_cast<void *>(handle), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK || stmt == nullptr) {
        LOG_ERROR("SQL prepare failed on '{}' (rc={}).", db, rc);
        return rows;
    }

    while (api.step(stmt) == SQLITE_ROW) {
        DbRow_t row;
        int col_count = api.column_count(stmt);
        for (int i = 0; i < col_count; ++i) {
            DbField_t field;
            field.type          = api.column_type(stmt, i);
            const char *colName = api.column_name(stmt, i);
            field.column        = (colName != nullptr) ? colName : "";

            if (field.type != SQLITE_NULL) {
                int length = api.column_bytes(stmt, i);
                if (length > 0) {
                    const uint8_t *data = (field.type == SQLITE_BLOB)
                                              ? reinterpret_cast<const uint8_t *>(api.column_blob(stmt, i))
                                              : reinterpret_cast<const uint8_t *>(api.column_text(stmt, i));
                    if (data != nullptr) {
                        field.content.assign(data, data + length);
                    }
                }
            }

            row.push_back(field);
        }
        rows.push_back(row);
    }

    api.finalize(stmt);
    return rows;
}

int get_local_id_and_dbidx(uint64_t id, uint64_t *local_id, uint32_t *db_idx)
{
    if ((local_id == nullptr) || (db_idx == nullptr)) {
        return -1;
    }

    for (const auto &[idx, dbname] : find_dbs_by_prefix("MSG")) {
        std::string sql = "SELECT localId FROM MSG WHERE MsgSvrID=" + std::to_string(id) + " LIMIT 1;";
        DbRows_t rows   = exec_db_query(dbname, sql);
        if (rows.empty() || rows.front().empty()) {
            continue;
        }

        std::string value = field_to_string(rows.front().front());
        if (value.empty()) {
            continue;
        }

        *local_id = _strtoui64(value.c_str(), nullptr, 10);
        *db_idx   = static_cast<uint32_t>(idx);
        return 0;
    }

    return -1;
}

std::vector<uint8_t> get_audio_data(uint64_t msg_id)
{
    for (const auto &[idx, dbname] : find_dbs_by_prefix("MediaMSG")) {
        (void)idx;
        std::string sql = "SELECT Buf FROM Media WHERE Reserved0=" + std::to_string(msg_id) + " LIMIT 1;";
        DbRows_t rows   = exec_db_query(dbname, sql);
        if (rows.empty() || rows.front().empty()) {
            continue;
        }

        const DbField_t &field = rows.front().front();
        if ((field.column != "Buf") || field.content.size() <= 1) {
            continue;
        }

        return std::vector<uint8_t>(field.content.begin() + 1, field.content.end());
    }

    return {};
}

bool rpc_get_db_names(uint8_t *out, size_t *len)
{
    // 数据须活到 fill_response 内的 pb_encode（assign 返回后才编码），故置于此作用域。
    DbNames_t dbnames = get_db_names();
    return fill_response<Functions_FUNC_GET_DB_NAMES>(out, len, [&dbnames](Response &rsp) {
        rsp.msg.dbs.names.funcs.encode = encode_dbnames;
        rsp.msg.dbs.names.arg          = &dbnames;
    });
}

bool rpc_get_db_tables(const std::string &db, uint8_t *out, size_t *len)
{
    DbTables_t tables = get_db_tables(db);
    return fill_response<Functions_FUNC_GET_DB_TABLES>(out, len, [&tables](Response &rsp) {
        rsp.msg.tables.tables.funcs.encode = encode_tables;
        rsp.msg.tables.tables.arg          = &tables;
    });
}

bool rpc_exec_db_query(const DbQuery &query, uint8_t *out, size_t *len)
{
    DbRows_t rows;
    if ((query.db == nullptr) || (query.sql == nullptr)) {
        LOG_ERROR("Empty db or sql.");
    } else {
        rows = exec_db_query(query.db, query.sql);
    }
    return fill_response<Functions_FUNC_EXEC_DB_QUERY>(out, len, [&rows](Response &rsp) {
        rsp.msg.rows.rows.funcs.encode = encode_rows;
        rsp.msg.rows.rows.arg          = &rows;
    });
}

} // namespace db
