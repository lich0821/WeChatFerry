#pragma once

#include "rpc_h.h"

RPC_STATUS RpcConnectServer();
RPC_STATUS RpcDisconnectServer();

int RpcEnableReceiveMsg();
int RpcDisableReceiveMsg();
int RpcIsLogin();
int RpcSendTextMsg(const wchar_t *wxid, const wchar_t *msg, const wchar_t *atWxids);
int RpcSendImageMsg(const wchar_t *wxid, const wchar_t *path);
PPRpcIntBstrPair RpcGetMsgTypes(int *pNum);
PPRpcContact RpcGetContacts(int *pNum);
BSTR *RpcGetDbNames(int *pNum);
PPRpcTables RpcGetDbTables(const wchar_t *db, int *pNum);
PPPRpcSqlResult RpcExecDbQuery(const wchar_t *db, const wchar_t *sql, int *row, int *col);
