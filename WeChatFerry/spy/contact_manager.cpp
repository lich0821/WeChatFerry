#pragma execution_character_set("utf-8")

#include "contact_manager.h"

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "database_executor.h"
#include "log.hpp"
#include "pb_util.h"
#include "rpc_helper.h"
#include "spy.h"
#include "spy_types.h"
#include "util.h"
#include "offsets.h"

namespace
{

struct WxStringValue {
    const wchar_t *wptr;
    uint32_t size;
    uint32_t capacity;
    const char *ptr;
    uint32_t clen;
};

using BufferInitFn        = void(__thiscall *)(void *buffer);
using BufferCleanupFn     = void(__thiscall *)(void *buffer);
using WxStringAssignFn    = void *(__thiscall *)(void *dest, const wchar_t *src, int len);
using AcceptNewFriendFn   = int(__thiscall *)(void *buffer, const WxString *v3, void *nullbuffer,
                                              int reserved1, uint64_t scratch, WxStringValue v4,
                                              int scene, int reserved2);

WxStringValue to_value(const WxString &value)
{
    return { value.wptr, value.size, value.capacity, value.ptr, value.clen };
}

std::string field_to_string(const DbField_t &field)
{
    return std::string(field.content.begin(), field.content.end());
}

std::set<std::string> get_contact_columns()
{
    std::set<std::string> columns;
    DbRows_t rows = db::exec_db_query("MicroMsg.db", "PRAGMA table_info(Contact);");
    for (const auto &row : rows) {
        for (const auto &field : row) {
            if (field.column == "name") {
                columns.insert(field_to_string(field));
            }
        }
    }
    return columns;
}

bool has_column(const std::set<std::string> &columns, const char *name)
{
    return columns.find(name) != columns.end();
}

std::string select_expr(const std::set<std::string> &columns, const char *source, const char *alias, bool numeric = false)
{
    if (has_column(columns, source)) {
        return std::string(source) + " AS " + alias;
    }
    return numeric ? ("0 AS " + std::string(alias)) : ("'' AS " + std::string(alias));
}

std::string escape_sql_string(const std::string &value)
{
    std::string escaped;
    escaped.reserve(value.size());
    for (char ch : value) {
        escaped.push_back(ch);
        if (ch == '\'') {
            escaped.push_back('\'');
        }
    }
    return escaped;
}

std::string build_contact_query(const std::string *wxid)
{
    std::set<std::string> columns = get_contact_columns();
    if (!has_column(columns, "UserName")) {
        return "";
    }

    std::vector<std::string> fields = {
        select_expr(columns, "UserName", "wxid"),
        select_expr(columns, "Alias", "code"),
        select_expr(columns, "Remark", "remark"),
        select_expr(columns, "NickName", "name"),
        select_expr(columns, "Sex", "gender", true),
        select_expr(columns, "Country", "country"),
        select_expr(columns, "Province", "province"),
        select_expr(columns, "City", "city"),
    };

    std::string sql = "SELECT ";
    for (size_t i = 0; i < fields.size(); ++i) {
        if (i != 0) {
            sql += ", ";
        }
        sql += fields[i];
    }
    sql += " FROM Contact";

    if (wxid != nullptr) {
        sql += " WHERE UserName='";
        sql += escape_sql_string(*wxid);
        sql += "'";
    }

    return sql + ";";
}

RpcContact_t row_to_contact(const DbRow_t &row)
{
    RpcContact_t contact = {};
    for (const auto &field : row) {
        std::string value = field_to_string(field);
        if (field.column == "wxid") {
            contact.wxid = value;
        } else if (field.column == "code") {
            contact.code = value;
        } else if (field.column == "remark") {
            contact.remark = value;
        } else if (field.column == "name") {
            contact.name = value;
        } else if (field.column == "country") {
            contact.country = value;
        } else if (field.column == "province") {
            contact.province = value;
        } else if (field.column == "city") {
            contact.city = value;
        } else if (field.column == "gender") {
            contact.gender = value.empty() ? 0 : atoi(value.c_str());
        }
    }

    return contact;
}

} // namespace

namespace contact
{

std::vector<RpcContact_t> get_contacts()
{
    std::vector<RpcContact_t> contacts;

    // 依据 MicroMsg.db 的 Contact 表实际列动态拼 SQL（缺列以 '' / 0 占位）
    std::string sql = build_contact_query(nullptr);
    if (sql.empty()) {
        LOG_ERROR("Failed to build contact query (missing UserName column?).");
        return contacts;
    }

    DbRows_t rows = db::exec_db_query("MicroMsg.db", sql);
    contacts.reserve(rows.size());
    for (const auto &row : rows) {
        contacts.push_back(row_to_contact(row));
    }
    return contacts;
}

int accept_new_friend(const std::string &v3, const std::string &v4, int scene)
{
    if (g_WeChatWinDllAddr == 0) {
        LOG_ERROR("WeChatWin.dll not located.");
        return -1;
    }

    // v3（加密 username）仅被读取（拷入 this+24），用非拥有视图即可；
    // v4（ticket）会被 VerifyOK mm_free，须用 ASSIGN 建 WeChat 拥有副本。
    auto ctor      = reinterpret_cast<BufferInitFn>(g_WeChatWinDllAddr + Offsets::Friend::ACCEPT_CTOR);
    auto verify_ok = reinterpret_cast<AcceptNewFriendFn>(g_WeChatWinDllAddr + Offsets::Friend::VERIFY_OK);
    auto dtor      = reinterpret_cast<BufferCleanupFn>(g_WeChatWinDllAddr + Offsets::Friend::ACCEPT_DTOR);
    auto assign    = reinterpret_cast<WxStringAssignFn>(g_WeChatWinDllAddr + Offsets::RichText::ASSIGN);

    LOG_DEBUG("accept_new_friend v3: {}, v4: {}, scene: {}", v3, v4, scene);

    std::wstring wsV3 = util::s2w(v3);
    std::wstring wsV4 = util::s2w(v4);
    WxString wxV3(wsV3);                       // 非拥有视图：VerifyOK 只读取
    WxString wxV4;                             // ticket：须为 WeChat 拥有副本（VerifyOK 末尾 mm_free 它）
    assign(&wxV4, wsV4.c_str(), -1);

    char buffer[0x40]      = { 0 };
    char nullbuffer[0x3CC] = { 0 };

    ctor(buffer);
    int success = verify_ok(buffer, &wxV3, nullbuffer, 0, 0, to_value(wxV4), scene, 0);
    dtor(buffer);

    return success;
}

int add_friend_by_wxid(const std::string &wxid, const std::string &msg)
{
    (void)wxid;
    (void)msg;
    return 0;
}

RpcContact_t get_contact_by_wxid(const std::string &wxid)
{
    RpcContact_t contact = {};
    contact.wxid         = wxid;

    // 与 get_contacts 同一条链，仅带 WHERE UserName=? 过滤单个联系人。
    std::string sql = build_contact_query(&wxid);
    if (sql.empty()) {
        LOG_ERROR("Failed to build contact query (missing UserName column?).");
        return contact;
    }

    DbRows_t rows = db::exec_db_query("MicroMsg.db", sql);
    if (!rows.empty()) {
        contact = row_to_contact(rows.front());
    }
    return contact;
}

bool rpc_get_contacts(uint8_t *out, size_t *len)
{
    std::vector<RpcContact_t> contacts = get_contacts();
    return fill_response<Functions_FUNC_GET_CONTACTS>(out, len, contacts, [](Response &rsp, auto &contacts) {
        rsp.msg.contacts.contacts.funcs.encode = encode_contacts;
        rsp.msg.contacts.contacts.arg          = &contacts;
    });
}

bool rpc_get_contact_info(const std::string &wxid, uint8_t *out, size_t *len)
{
    std::vector<RpcContact_t> contacts;
    if (!wxid.empty()) {
        contacts.push_back(get_contact_by_wxid(wxid));
    }
    return fill_response<Functions_FUNC_GET_CONTACT_INFO>(out, len, contacts, [](Response &rsp, auto &contacts) {
        rsp.msg.contacts.contacts.funcs.encode = encode_contacts;
        rsp.msg.contacts.contacts.arg          = &contacts;
    });
}

bool rpc_accept_friend(const std::string &v3, const std::string &v4, int scene, uint8_t *out, size_t *len)
{
    int result = accept_new_friend(v3, v4, scene);
    return fill_response<Functions_FUNC_ACCEPT_FRIEND>(out, len, [result](Response &rsp) {
        rsp.msg.status = result;
    });
}

} // namespace contact
