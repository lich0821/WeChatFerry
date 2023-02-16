#pragma once

#include <map>
#include <string>

typedef std::map<int, std::string> MsgTypes_t;

MsgTypes_t GetMsgTypes();

#if 0
#include "../proto/wcf.grpc.pb.h"

void ListenMessage();
void UnListenMessage();
void GetMsgTypes(wcf::MsgTypes *types);
#endif