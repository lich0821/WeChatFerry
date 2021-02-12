#pragma once

#define MSG_SIZE_MSG_ID  64
#define MSG_SIZE_MSG_XML 4096
#define MSG_SIZE_WXID    64
#define MSG_SIZE_ROOMID  64
#define MSG_SIZE_CONTENT 16385

typedef struct RpcMessage {
    int self;                          // 是否自己发的消息：0=否，1=是
    int type;                          // 消息类型
    int source;                        // 消息来源：0=好友消息，1=群消息
    wchar_t id[MSG_SIZE_MSG_ID];       // 消息ID
    wchar_t xml[MSG_SIZE_MSG_XML];     // 群其他消息
    wchar_t wxId[MSG_SIZE_WXID];       // 发送人微信ID
    wchar_t roomId[MSG_SIZE_ROOMID];   // 群ID
    wchar_t content[MSG_SIZE_CONTENT]; // 消息内容，MAC版最大：16384，即16KB
} RpcMessage_t;
