#pragma once

#include "../proto/wcf.grpc.pb.h"

void ListenMessage();
void UnListenMessage();
void GetMsgTypes(wcf::MsgTypes *types);
