#include "database_executor.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <string>
#include <system_error>
#include <vector>

#include <ShlObj.h>

#include "account_manager.h"
#include "log.hpp"
#include "pb_util.h"
#include "rpc_helper.h"
#include "util.h"

#pragma comment(lib, "Shell32.lib")

namespace fs = std::filesystem;

namespace
{

struct sqlite3;
struct sqlite3_stmt;

constexpr int SQLITE_OK      = 0;
constexpr int SQLITE_ROW     = 100;
constexpr int SQLITE_INTEGER = 1;
constexpr int SQLITE_FLOAT   = 2;
constexpr int SQLITE_TEXT    = 3;
constexpr int SQLITE_BLOB    = 4;
constexpr int SQLITE_NULL    = 5;

using sqlite3_open16_fn       = int(__cdecl *)(const void *filename, sqlite3 **ppDb);
using sqlite3_close_fn        = int(__cdecl *)(sqlite3 *db);
using sqlite3_prepare_v2_fn   = int(__cdecl *)(sqlite3 *db, const char *sql, int nByte, sqlite3_stmt **ppStmt,
                                             const char **pzTail);
using sqlite3_step_fn         = int(__cdecl *)(sqlite3_stmt *stmt);
using sqlite3_finalize_fn     = int(__cdecl *)(sqlite3_stmt *stmt);
using sqlite3_column_count_fn = int(__cdecl *)(sqlite3_stmt *stmt);
using sqlite3_column_name_fn  = const char *(__cdecl *)(sqlite3_stmt *stmt, int iCol);
using sqlite3_column_type_fn  = int(__cdecl *)(sqlite3_stmt *stmt, int iCol);
using sqlite3_column_blob_fn  = const void *(__cdecl *)(sqlite3_stmt *stmt, int iCol);
using sqlite3_column_text_fn  = const unsigned char *(__cdecl *)(sqlite3_stmt *stmt, int iCol);
using sqlite3_column_bytes_fn = int(__cdecl *)(sqlite3_stmt *stmt, int iCol);

struct SqliteApi {
    HMODULE module               = nullptr;
    sqlite3_open16_fn open16     = nullptr;
    sqlite3_close_fn close       = nullptr;
    sqlite3_prepare_v2_fn prepare = nullptr;
    sqlite3_step_fn step         = nullptr;
    sqlite3_finalize_fn finalize = nullptr;
    sqlite3_column_count_fn column_count = nullptr;
    sqlite3_column_name_fn column_name   = nullptr;
    sqlite3_column_type_fn column_type   = nullptr;
    sqlite3_column_blob_fn column_blob   = nullptr;
    sqlite3_column_text_fn column_text   = nullptr;
    sqlite3_column_bytes_fn column_bytes = nullptr;

    bool init()
    {
        if (module != nullptr) {
            return true;
        }

        module = LoadLibraryW(L"winsqlite3.dll");
        if (module == nullptr) {
            LOG_ERROR("Failed to load winsqlite3.dll");
            return false;
        }

        open16       = reinterpret_cast<sqlite3_open16_fn>(GetProcAddress(module, "sqlite3_open16"));
        close        = reinterpret_cast<sqlite3_close_fn>(GetProcAddress(module, "sqlite3_close"));
        prepare      = reinterpret_cast<sqlite3_prepare_v2_fn>(GetProcAddress(module, "sqlite3_prepare_v2"));
        step         = reinterpret_cast<sqlite3_step_fn>(GetProcAddress(module, "sqlite3_step"));
        finalize     = reinterpret_cast<sqlite3_finalize_fn>(GetProcAddress(module, "sqlite3_finalize"));
        column_count = reinterpret_cast<sqlite3_column_count_fn>(GetProcAddress(module, "sqlite3_column_count"));
        column_name  = reinterpret_cast<sqlite3_column_name_fn>(GetProcAddress(module, "sqlite3_column_name"));
        column_type  = reinterpret_cast<sqlite3_column_type_fn>(GetProcAddress(module, "sqlite3_column_type"));
        column_blob  = reinterpret_cast<sqlite3_column_blob_fn>(GetProcAddress(module, "sqlite3_column_blob"));
        column_text  = reinterpret_cast<sqlite3_column_text_fn>(GetProcAddress(module, "sqlite3_column_text"));
        column_bytes = reinterpret_cast<sqlite3_column_bytes_fn>(GetProcAddress(module, "sqlite3_column_bytes"));

        if ((open16 == nullptr) || (close == nullptr) || (prepare == nullptr) || (step == nullptr) ||
            (finalize == nullptr) || (column_count == nullptr) || (column_name == nullptr) ||
            (column_type == nullptr) || (column_blob == nullptr) || (column_text == nullptr) ||
            (column_bytes == nullptr)) {
            LOG_ERROR("Failed to resolve winsqlite3 symbols");
            FreeLibrary(module);
            module = nullptr;
            return false;
        }

        return true;
    }
};

SqliteApi &sqlite_api()
{
    static SqliteApi api;
    return api;
}

class SqliteDb
{
  public:
    SqliteDb() = default;

    ~SqliteDb()
    {
        if (db_ != nullptr) {
            sqlite_api().close(db_);
        }
    }

    SqliteDb(const SqliteDb &)            = delete;
    SqliteDb &operator=(const SqliteDb &) = delete;

    bool open(const std::wstring &path)
    {
        if (!sqlite_api().init()) {
            return false;
        }

        return sqlite_api().open16(path.c_str(), &db_) == SQLITE_OK;
    }

    sqlite3 *get() const
    {
        return db_;
    }

  private:
    sqlite3 *db_ = nullptr;
};

class SqliteStmt
{
  public:
    explicit SqliteStmt(sqlite3_stmt *stmt) : stmt_(stmt) {}

    ~SqliteStmt()
    {
        if (stmt_ != nullptr) {
            sqlite_api().finalize(stmt_);
        }
    }

    SqliteStmt(const SqliteStmt &)            = delete;
    SqliteStmt &operator=(const SqliteStmt &) = delete;

    sqlite3_stmt *get() const
    {
        return stmt_;
    }

  private:
    sqlite3_stmt *stmt_ = nullptr;
};

using db_map_t = std::map<std::string, std::wstring>;

db_map_t db_map;

std::wstring get_default_wechat_root()
{
    wchar_t path[MAX_PATH] = { 0 };
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, path))) {
        return std::wstring(path) + L"\\WeChat Files\\";
    }
    return L"";
}

std::vector<std::wstring> build_search_roots()
{
    std::vector<std::wstring> roots;

    std::string home = account::get_home_path();
    if (home.empty()) {
        home = util::w2s(get_default_wechat_root());
    }

    if (home.empty()) {
        return roots;
    }

    std::wstring whome = util::s2w(home);
    std::string wxid   = account::get_self_wxid();
    if (!wxid.empty()) {
        roots.push_back(whome + util::s2w(wxid));
    }
    roots.push_back(whome);

    return roots;
}

void refresh_db_map()
{
    db_map.clear();

    for (const auto &root : build_search_roots()) {
        if (root.empty()) {
            continue;
        }

        std::error_code ec;
        if (!fs::exists(root, ec)) {
            continue;
        }

        fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, ec);
        fs::recursive_directory_iterator end;
        for (; !ec && it != end; it.increment(ec)) {
            if (ec || !it->is_regular_file(ec)) {
                continue;
            }

            const fs::path &path = it->path();
            if (path.extension() != L".db") {
                continue;
            }

            std::string name = util::w2s(path.filename().wstring());
            db_map.emplace(name, path.wstring());
        }
    }
}

const std::wstring *find_db_path(const std::string &db)
{
    if (db_map.empty()) {
        refresh_db_map();
    }

    auto it = db_map.find(db);
    if (it != db_map.end()) {
        return &it->second;
    }

    refresh_db_map();
    it = db_map.find(db);
    return it == db_map.end() ? nullptr : &it->second;
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
    for (const auto &[name, path] : db_map) {
        (void)path;
        int idx = parse_db_index(name, prefix);
        if (idx >= 0) {
            result.emplace_back(idx, name);
        }
    }

    std::sort(result.begin(), result.end(), [](const auto &lhs, const auto &rhs) { return lhs.first > rhs.first; });
    return result;
}

DbRows_t query_rows(const std::wstring &path, const std::string &sql)
{
    DbRows_t rows;

    SqliteDb db;
    if (!db.open(path)) {
        LOG_ERROR("Failed to open database: {}", util::w2s(path));
        return rows;
    }

    sqlite3_stmt *stmt = nullptr;
    int rc             = sqlite_api().prepare(db.get(), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK || stmt == nullptr) {
        LOG_ERROR("Failed to prepare SQL on {}", util::w2s(path));
        return rows;
    }

    SqliteStmt holder(stmt);

    while (sqlite_api().step(stmt) == SQLITE_ROW) {
        DbRow_t row;
        int col_count = sqlite_api().column_count(stmt);
        for (int i = 0; i < col_count; ++i) {
            DbField_t field;
            field.type   = sqlite_api().column_type(stmt, i);
            field.column = sqlite_api().column_name(stmt, i);

            if (field.type == SQLITE_NULL) {
                row.push_back(field);
                continue;
            }

            int length = sqlite_api().column_bytes(stmt, i);
            if (length <= 0) {
                row.push_back(field);
                continue;
            }

            if (field.type == SQLITE_BLOB) {
                const auto *blob = reinterpret_cast<const uint8_t *>(sqlite_api().column_blob(stmt, i));
                field.content.assign(blob, blob + length);
            } else {
                const auto *text = reinterpret_cast<const uint8_t *>(sqlite_api().column_text(stmt, i));
                field.content.assign(text, text + length);
            }

            row.push_back(field);
        }
        rows.push_back(row);
    }

    return rows;
}

} // namespace

namespace db
{

DbNames_t get_db_names()
{
    LOG_ERROR("Not Implemented yet.");
    DbNames_t names;
    return names;
}

DbTables_t get_db_tables(const std::string &db)
{
    (void)db;
    LOG_ERROR("Not Implemented yet.");
    DbTables_t tables;
    return tables;
}

DbRows_t exec_db_query(const std::string &db, const std::string &sql)
{
    (void)db;
    (void)sql;
    LOG_ERROR("Not Implemented yet.");
    return {};
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
        if ((query.db == nullptr) || (query.sql == nullptr)) {
            LOG_ERROR("Empty db or sql.");
            DbRows_t rows;
            rsp.msg.rows.rows.funcs.encode = encode_rows;
            rsp.msg.rows.rows.arg          = &rows;
        } else {
            DbRows_t rows = exec_db_query(query.db, query.sql);
            rsp.msg.rows.rows.funcs.encode = encode_rows;
            rsp.msg.rows.rows.arg          = &rows;
        }
    });
}

} // namespace db
