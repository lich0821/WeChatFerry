#pragma once

#include <map>
#include <string>

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
