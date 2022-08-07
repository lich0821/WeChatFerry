#pragma once

#include "rpc_h.h"

RPC_STATUS RpcConnectServer();
RPC_STATUS RpcDisconnectServer();

unsigned int __stdcall RpcSetTextMsgCb(void *p);
int RpcIsLogin();
int RpcSendTextMsg(const wchar_t *wxid, const wchar_t *at_wxid, const wchar_t *msg);
int RpcSendImageMsg(const wchar_t *wxid, const wchar_t *path);
