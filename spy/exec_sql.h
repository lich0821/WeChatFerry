#pragma once
#if 0
#include <string>
#include <vector>

#include "../proto/wcf.grpc.pb.h"

void GetDbNames(wcf::DbNames *names);
void GetDbTables(const std::string db, wcf::DbTables *tables);
void ExecDbQuery(const std::string db, const std::string sql, wcf::DbRows *rows);
#endif