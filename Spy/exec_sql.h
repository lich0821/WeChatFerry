#pragma once

#include <string>
#include <vector>

#include "rpc_h.h"

std::vector<std::wstring> GetDbNames();
std::vector<RpcTables_t> GetDbTables(std::wstring db);
std::vector<std::vector<RpcSqlResult_t>> ExecDbQuery(std::wstring db, std::wstring sql);
