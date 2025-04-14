#pragma once

#include "pb_types.h"

void EnableLog();
void DisableLog();
void ListenPyq();
void UnListenPyq();
void ListenMessage();
void UnListenMessage();
MsgTypes_t GetMsgTypes();
