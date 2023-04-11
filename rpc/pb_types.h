#pragma once

#include <map>
#include <string>
#include <vector>

using namespace std;

typedef map<int, string> MsgTypes_t;

typedef struct {
    int32_t gender;
    string wxid;
    string code;
    string name;
    string country;
    string province;
    string city;
} RpcContact_t;

typedef vector<string> DbNames_t;

typedef struct {
    string name;
    string sql;
} DbTable_t;
typedef vector<DbTable_t> DbTables_t;

typedef struct {
    int32_t type;
    string column;
    vector<uint8_t> content;
} DbField_t;
typedef vector<DbField_t> DbRow_t;
typedef vector<DbRow_t> DbRows_t;

typedef struct {
    bool is_self;
    bool is_group;
    int32_t type;
    string id;
    string xml;
    string sender;
    string roomid;
    string content;
    string thumb;
    string extra;
} WxMsg_t;

typedef struct {
    string wxid;
    string name;
    string mobile;
    string home;
} UserInfo_t;
