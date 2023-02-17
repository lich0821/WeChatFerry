#pragma once

#include "pb_types.h"

DbNames_t GetDbNames();
DbTables_t GetDbTables(const string db);
DbRows_t ExecDbQuery(const string db, const string sql);
