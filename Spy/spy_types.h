#pragma once

#include "framework.h"
#include <queue>

#include "rpc_h.h"

typedef struct UserInfoCall {
    DWORD wxid;
    DWORD nickName;
    DWORD mobile;
} UserInfoCall_t;

typedef struct RecvMsg {
    DWORD hook;    // Hook地址
    DWORD call;    // Call地址
    DWORD type;    // 消息类型地址
    DWORD isSelf;  // 是否自己发送标志地址
    DWORD msgId;   // 消息ID地址
    DWORD msgXml;  // 消息xml内容地址
    DWORD roomId;  // 群聊时，为群ID；私聊时，为微信ID
    DWORD wxId;    // 私聊时，为空；群聊时，为发送者微信ID
    DWORD content; // 消息内容地址
} RecvMsg_t;

typedef struct SendImg {
    DWORD call1;
    DWORD call2;
    DWORD call3;
} SendImg_t;

typedef struct Contact {
    DWORD base;
    DWORD head;
    DWORD wxId;
    DWORD wxCode;
    DWORD wxName;
    DWORD wxGender;
    DWORD wxCountry;
    DWORD wxProvince;
    DWORD wxCity;
} Contact_t;

typedef struct Sql {
    DWORD exec;
    DWORD base;
    DWORD start;
    DWORD end;
    DWORD slot;
    DWORD name;
} Sql_t;

typedef struct NewFriend {
    DWORD call1;
    DWORD call2;
    DWORD handle;
} NewFriend_t;

typedef struct WxCalls {
    DWORD login;       // 登录状态
    UserInfoCall_t ui; // 用户信息
    DWORD sendTextMsg; // 发送消息
    RecvMsg_t recvMsg; // 接收消息
    SendImg_t sendImg; // 发送图片
    Contact_t contact; // 获取联系人
    Sql_t sql;         // 执行 SQL
    NewFriend_t anf;   // 通过好友申请

} WxCalls_t;

typedef struct TextStruct {
    wchar_t *text;
    DWORD size;
    DWORD capacity;
    char fill[8];
} TextStruct_t;

typedef std::queue<RpcMessage_t> MsgQueue_t;
