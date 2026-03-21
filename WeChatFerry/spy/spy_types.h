#pragma once

#include "framework.h"
#include <string>

typedef struct UserInfoCall {
    uint32_t wxid;
    uint32_t nickName;
    uint32_t mobile;
    uint32_t home;
} UserInfoCall_t;

typedef struct RecvMsg {
    uint32_t hook;    // Hook地址
    uint32_t call;    // Call地址
    uint32_t msgId;   // 消息ID地址
    uint32_t type;    // 消息类型地址
    uint32_t isSelf;  // 是否自己发送标志地址
    uint32_t ts;      // TimeStamp
    uint32_t roomId;  // 群聊时，为群ID；私聊时，为微信ID
    uint32_t content; // 消息内容地址
    uint32_t wxid;    // 私聊时，为空；群聊时，为发送者微信ID
    uint32_t sign;    // Sign
    uint32_t thumb;   // 缩略图
    uint32_t extra;   // 附加数据
    uint32_t msgXml;  // 消息xml内容地址
} RecvMsg_t;

typedef struct SendText {
    uint32_t call1;
    uint32_t call2;
    uint32_t call3;
} SendText_t;

typedef struct Sendfile {
    uint32_t call1;
    uint32_t call2;
    uint32_t call3;
    uint32_t call4;
} Sendfile_t;

typedef struct Contact {
    uint32_t base;
    uint32_t head;
    uint32_t wxId;
    uint32_t wxCode;
    uint32_t wxRemark;
    uint32_t wxName;
    uint32_t wxGender;
    uint32_t wxCountry;
    uint32_t wxProvince;
    uint32_t wxCity;
} Contact_t;

typedef struct Sql {
    uint32_t exec;
    uint32_t base;
    uint32_t start;
    uint32_t end;
    uint32_t slot;
    uint32_t name;
} Sql_t;

typedef struct NewFriend {
    uint32_t call1;
    uint32_t call2;
    uint32_t call3;
    uint32_t call4;
} NewFriend_t;

typedef struct RoomMember {
    uint32_t call1;
    uint32_t call2;
    uint32_t call3;
} RoomMember_t;

typedef struct Xml {
    uint32_t call1;
    uint32_t call2;
    uint32_t call3;
    uint32_t call4;
    uint32_t param;
} Xml_t;

typedef struct TF {
    uint32_t call1;
    uint32_t call2;
    uint32_t call3;
} TF_t;

typedef struct Pyq {
    uint32_t hook;
    uint32_t call;
    uint32_t call1;
    uint32_t call2;
    uint32_t call3;
    uint32_t start;
    uint32_t end;
    uint32_t ts;
    uint32_t wxid;
    uint32_t content;
    uint32_t xml;
    uint32_t step;
} Pyq_t;

typedef struct DlAttach {
    uint32_t call1;
    uint32_t call2;
    uint32_t call3;
    uint32_t call4;
    uint32_t call5;
    uint32_t call6;
} DlAttach_t;

typedef struct RevokeMsg {
    uint32_t call1;
    uint32_t call2;
    uint32_t call3;
    uint32_t call4;
    uint32_t call5;
} RevokeMsg_t;

typedef struct CallRichText {
    uint32_t call1;
    uint32_t call2;
    uint32_t call3;
    uint32_t call4;
    uint32_t call5;
} CallRichText_t;

typedef struct CallPatMsg {
    uint32_t call1;
    uint32_t call2;
    uint32_t call3;
} CallPatMsg_t;

typedef struct CallInviteCM {
    uint32_t call1;
    uint32_t call2;
    uint32_t call3;
    uint32_t call4;
    uint32_t call5;
    uint32_t call6;
    uint32_t call7;
    uint32_t call8;
} CallInviteCM_t;

typedef struct CallOcr {
    uint32_t call1;
    uint32_t call2;
    uint32_t call3;
} CallOcr_t;

typedef struct CallFm {
    uint32_t call1;
    uint32_t call2;
} CallFm_t;

typedef struct CallRfLoginQr {
    uint32_t call1;
    uint32_t call2;
    uint32_t url;
} CallRfLoginQr_t;

typedef struct WxCalls {
    uint32_t login;         // 登录状态
    UserInfoCall_t ui;   // 用户信息
    SendText_t sendText; // 发送消息
    RecvMsg_t recvMsg;   // 接收消息
    Sendfile_t sendImg;  // 发送图片
    Sendfile_t sendFile; // 发送文件
    Xml_t sendXml;       // 发送XML
    Sendfile_t sendEmo;  // 发送表情
    Contact_t contact;   // 获取联系人
    Sql_t sql;           // 执行 SQL
    NewFriend_t anf;     // 通过好友申请
    RoomMember_t arm;    // 添加群成员
    RoomMember_t drm;    // 删除群成员
    TF_t tf;             // 接收转账
    Pyq_t pyq;           // 接收朋友圈消息
    DlAttach_t da;       // 下载资源（图片、文件、视频）
    RevokeMsg_t rm;      // 撤回消息
    CallRichText_t rt;   // 发送消息卡片
    CallPatMsg_t pm;     // 发送拍一拍消息
    CallInviteCM_t irm;  // 邀请群成员
    CallOcr_t ocr;       // OCR
    CallFm_t fm;         // 转发消息
    CallRfLoginQr_t rlq; // 刷新登录二维码
} WxCalls_t;

struct WxString {
    const wchar_t *wptr;
    uint32_t size;
    uint32_t capacity;
    const char *ptr;
    uint32_t clen;
    WxString()
    {
        wptr     = NULL;
        size     = 0;
        capacity = 0;
        ptr      = NULL;
        clen     = 0;
    }

    WxString(std::wstring &ws)
    {
        wptr     = ws.c_str();
        size     = ws.size();
        capacity = ws.capacity();
        ptr      = NULL;
        clen     = 0;
    }
};

typedef struct RawVector {
    uint32_t start;
    uint32_t finish;
    uint32_t end;
} RawVector_t;
