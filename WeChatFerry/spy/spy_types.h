#pragma once

#include "framework.h"
#include <string>

typedef uint64_t QWORD;
typedef struct UserInfoCall {
    DWORD wxid = 0x5AB7FB8;
    DWORD nickName = 0x5AB8098;
    DWORD mobile = 0x5AB7FD8;
    DWORD home = 0x5A7E190;
} UserInfoCall_t;

typedef struct RecvMsg {
    DWORD hook = 0x00;    // Hook地址
    DWORD call = 0x2205510;    // Call地址
    DWORD msgId = 0x30;   // 消息ID地址
    DWORD type = 0x38;    // 消息类型地址
    DWORD isSelf = 0x3C;  // 是否自己发送标志地址
    DWORD ts = 0x44;      // TimeStamp
    DWORD roomId = 0x48;  // 群聊时，为群ID；私聊时，为微信ID
    DWORD content = 0x88; // 消息内容地址
    DWORD wxid = 0x240;    // 私聊时，为空；群聊时，为发送者微信ID
    DWORD sign = 0x260;    // Sign
    DWORD thumb = 0x280;   // 缩略图
    DWORD extra = 0x2A0;   // 附加数据
    DWORD msgXml = 0x308;  // 消息xml内容地址
} RecvMsg_t;

typedef struct SendText {
    DWORD call1 = 0x1C1E690;
    DWORD call2 = 0x238DDD0;
    DWORD call3 = 0x1C1FF10;
};

typedef struct Sendfile {
    DWORD call1 = 0x1C28800;
    DWORD call2 = 0x1C1FF10;
    DWORD call3 = 0x1C23630;
    DWORD call4 = 0x21969E0;
};
typedef struct SendEmo {
    DWORD call1 = 0x771980;
    DWORD call2 = 0x4777E0;
    DWORD call3 = 0x239E888;
};


typedef struct SendImage {
    DWORD call1 = 0x1C28800;
    DWORD call2 = 0x1C1FF10;
    DWORD call3 = 0x1C1E690;
    DWORD call4 = 0x2383560;
};


typedef struct Contact {
    DWORD base;
    DWORD head;
    DWORD wxId;
    DWORD wxCode;
    DWORD wxRemark;
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
    DWORD call3;
    DWORD call4;
} NewFriend_t;

typedef struct AddRoomMember {
    DWORD call1;
    DWORD call2;
    DWORD call3;
} ;

typedef struct DelRoomMember {
    DWORD call1;
    DWORD call2;
    DWORD call3;
};

typedef struct Xml {
    DWORD call1;
    DWORD call2;
    DWORD call3;
    DWORD call4;
    DWORD param;
} Xml_t;

typedef struct TF {
    DWORD call1;
    DWORD call2;
    DWORD call3;
} TF_t;

typedef struct Pyq {
    DWORD hook;
    DWORD call;
    DWORD call1;
    DWORD call2;
    DWORD call3;
    DWORD start;
    DWORD end;
    DWORD ts;
    DWORD wxid;
    DWORD content;
    DWORD xml;
    DWORD step;
} Pyq_t;

typedef struct DlAttach {
    DWORD call1;
    DWORD call2;
    DWORD call3;
    DWORD call4;
    DWORD call5;
    DWORD call6;
} DlAttach_t;

typedef struct RevokeMsg {
    DWORD call1;
    DWORD call2;
    DWORD call3;
    DWORD call4;
    DWORD call5;
} RevokeMsg_t;

typedef struct CallRichText {
    DWORD call1;
    DWORD call2;
    DWORD call3;
    DWORD call4;
} CallRichText_t;

typedef struct CallPatMsg {
    DWORD call1;
    DWORD call2;
    DWORD call3;
} CallPatMsg_t;

typedef struct CallInviteCM {
    DWORD call1;
    DWORD call2;
    DWORD call3;
    DWORD call4;
    DWORD call5;
    DWORD call6;
    DWORD call7;
    DWORD call8;
} CallInviteCM_t;

typedef struct CallOcr {
    DWORD call1;
    DWORD call2;
    DWORD call3;
} CallOcr_t;

typedef struct CallFm {
    DWORD call1;
    DWORD call2;
} CallFm_t;

typedef struct CallRfLoginQr {
    DWORD call1;
    DWORD call2;
    DWORD url;
} CallRfLoginQr_t;

typedef struct WxCalls {
    //DWORD login = 0x5AB8A2C;         // 登录状态
    //UserInfoCall_t ui;   // 用户信息
    //SendText sendText; // 发送消息
    //RecvMsg_t recvMsg;   // 接收消息
    //SendImage sendImg;  // 发送图片
    //Sendfile sendFile; // 发送文件
    //Xml_t sendXml;       // 发送XML
    //SendEmo sendEmo;  // 发送表情
    //Contact_t contact;   // 获取联系人
    //Sql_t sql;           // 执行 SQL
    //NewFriend_t anf;     // 通过好友申请
    //AddRoomMember arm;    // 添加群成员
    //DelRoomMember drm;    // 删除群成员
    //TF_t tf;             // 接收转账
    //Pyq_t pyq;           // 接收朋友圈消息
    //DlAttach_t da;       // 下载资源（图片、文件、视频）
    //RevokeMsg_t rm;      // 撤回消息
    //CallRichText_t rt;   // 发送消息卡片
    //CallPatMsg_t pm;     // 发送拍一拍消息
    //CallInviteCM_t irm;  // 邀请群成员
    //CallOcr_t ocr;       // OCR
    //CallFm_t fm;         // 转发消息
    //CallRfLoginQr_t rlq; // 刷新登录二维码
} WxCalls_t;

struct WxString {
    const wchar_t *wptr;
    DWORD size;
    DWORD capacity;
    const char *ptr;
    DWORD clen;
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
        size     = (DWORD)ws.size();
        capacity = (DWORD)ws.capacity();
        ptr      = NULL;
        clen     = 0;
    }
};

typedef struct RawVector {
#ifdef _DEBUG
    QWORD head;
#endif
    QWORD start;
    QWORD finish;
    QWORD end;
} RawVector_t;
