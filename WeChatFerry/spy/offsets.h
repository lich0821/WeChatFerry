#pragma once

#include <cstdint>

namespace Offsets
{

namespace Account
{
    constexpr uint64_t SERVICE = 0x1B5CA40; // 账户服务
    constexpr uint64_t PATH    = 0x25F4A40; // 数据路径
    constexpr uint64_t WXID    = 0x80;      // WXID
    constexpr uint64_t NAME    = 0x1E8;     // 昵称
    constexpr uint64_t MOBILE  = 0x128;     // 手机号
    constexpr uint64_t LOGIN   = 0x7F8;     // 登录状态
    constexpr uint64_t ALIAS   = 0x108;     // 修改后的WXID
}

namespace Chatroom
{
    constexpr uint64_t MGR = 0x1B8AE40;
    constexpr uint64_t NEW = 0x262D800;
    constexpr uint64_t DEL = 0x2163070;
    constexpr uint64_t ADD = 0x2162A30;
    constexpr uint64_t INV = 0x2162410;
}

namespace Contact
{
    constexpr uint64_t MGR     = 0x1B489D0;
    constexpr uint64_t LIST    = 0x21ACBE0;
    constexpr uint64_t BIN     = 0x200;
    constexpr uint64_t BIN_LEN = 0x208;
    constexpr uint64_t WXID    = 0x10;
    constexpr uint64_t CODE    = 0x30;
    constexpr uint64_t REMARK  = 0x80;
    constexpr uint64_t NAME    = 0xA0;
    constexpr uint64_t GENDER  = 0x0E;
    constexpr uint64_t STEP    = 0x6A8;

    constexpr uint64_t VERIFY_NEW        = Chatroom::NEW;
    constexpr uint64_t VERIFY_OK         = 0x1F48850;
    constexpr uint64_t ADD_FRIEND_HELPER = 0x4F7FB18;  // a1
    constexpr uint64_t FVDF              = 0x4F9DE28;  // FriendVeriyDialogFragment
}

namespace Db
{
    constexpr uint64_t INSTANCE     = 0x59D2008; // 数据库实例地址
    constexpr uint64_t MSG_I        = 0x5A30158; // MSGi.db & MediaMsgi.db
    constexpr uint64_t MICROMSG     = 0xB8;
    constexpr uint64_t CHAT_MSG     = 0x2C8;
    constexpr uint64_t MISC         = 0x5F0;
    constexpr uint64_t EMOTION      = 0x15F0;
    constexpr uint64_t MEDIA        = 0xF48;
    constexpr uint64_t BIZCHAT_MSG  = 0x1AC0;
    constexpr uint64_t FUNCTION_MSG = 0x1B98;
    constexpr uint64_t NAME         = 0x28;

    // SQLITE3
    constexpr uint64_t EXEC = 0x3A820A0;
    // constexpr uint64_t BACKUP_INIT      = EXEC - 0x1D113E0;
    constexpr uint64_t PREPARE = EXEC + 0x7CB0;
    // constexpr uint64_t OPEN             = EXEC - 0x1CA2430;
    // constexpr uint64_t BACKUP_STEP      = EXEC - 0x1D110A0;
    // constexpr uint64_t BACKUP_REMAINING = EXEC - 0x1D10880;
    // constexpr uint64_t BACKUP_PAGECOUNT = EXEC - 0x1D10890;
    // constexpr uint64_t BACKUP_FINISH    = EXEC - 0x1D10940;
    // constexpr uint64_t SLEEP            = EXEC - 0x1CA1BB0;
    // constexpr uint64_t ERRCODE          = EXEC - 0x1CA3770;
    // constexpr uint64_t CLOSE            = EXEC - 0x1CA4FD0;
    constexpr uint64_t STEP         = EXEC - 0x3C000;
    constexpr uint64_t COLUMN_COUNT = EXEC - 0x3B7E0;
    constexpr uint64_t COLUMN_NAME  = EXEC - 0x3ADE0;
    constexpr uint64_t COLUMN_TYPE  = EXEC - 0x3AF90;
    constexpr uint64_t COLUMN_BLOB  = EXEC - 0x3B7B0;
    constexpr uint64_t COLUMN_BYTES = EXEC - 0x3B6C0;
    constexpr uint64_t FINALIZE     = EXEC - 0x3CF50;
}

namespace Message
{
    namespace Log
    {
        constexpr uint64_t LEVEL = 0x578DF28; // 日志级别
        constexpr uint64_t CALL  = 0x2627590; // 日志函数
    }

    namespace Receive
    {
        constexpr uint64_t CALL      = 0x214C6C0; // 接收消息 Call
        constexpr uint64_t ID        = 0x30;      // 消息 ID
        constexpr uint64_t TYPE      = 0x38;      // 消息类型
        constexpr uint64_t SELF      = 0x3C;      // 消息是否来自自己
        constexpr uint64_t TIMESTAMP = 0x44;      // 消息时间戳
        constexpr uint64_t ROOMID    = 0x48;      // 群聊 ID（或者发送者 wxid）
        constexpr uint64_t CONTENT   = 0x88;      // 消息内容
        constexpr uint64_t WXID      = 0x240;     // 发送者 wxid
        constexpr uint64_t SIGN      = 0x260;     // 消息签名
        constexpr uint64_t THUMB     = 0x280;     // 缩略图路径
        constexpr uint64_t EXTRA     = 0x2A0;     // 原图路径
        constexpr uint64_t XML       = 0x308;     // 消息 XML

        constexpr uint64_t PYQ_CALL    = 0x2E621D0; // 接收朋友圈 Call
        constexpr uint64_t PYQ_START   = 0x30;      // 开始地址
        constexpr uint64_t PYQ_END     = 0x38;      // 结束地址
        constexpr uint64_t PYQ_SENDER  = 0x18;      // 发布者
        constexpr uint64_t PYQ_TS      = 0x38;      // 时间戳
        constexpr uint64_t PYQ_CONTENT = 0x48;      // 文本内容
        constexpr uint64_t PYQ_XML     = 0x9B8;     // 其他内容
    }

    namespace Send
    {
        constexpr uint64_t MGR          = 0x1B5B210;
        constexpr uint64_t INSTANCE     = 0x1B653B0;
        constexpr uint64_t FREE         = 0x1B5CAC0;
        constexpr uint64_t TEXT         = 0x22D4A90;
        constexpr uint64_t IMAGE        = 0x22CA2A0;
        constexpr uint64_t APP_MGR      = 0x1B601E0;
        constexpr uint64_t FILE         = 0x20DE200;
        constexpr uint64_t XML          = 0x20DD330;
        constexpr uint64_t XML_BUF_SIGN = 0x2503760;
        constexpr uint64_t EMOTION_MGR  = 0x1BD6300;
        constexpr uint64_t EMOTION      = 0x21C2EE0;

        constexpr uint64_t NEW_MM_READER  = 0x1B64900;
        constexpr uint64_t FREE_MM_READER = 0x1B63CD0;
        constexpr uint64_t RICH_TEXT      = 0x20E81E0;

        constexpr uint64_t PAT = 0x2CCDDC0;

        constexpr uint64_t FORWARD = 0x22D4010;
    }
}

namespace Misc
{
    constexpr uint64_t QR_CODE = 0x202D3C0;

    constexpr uint64_t INSATNCE         = Message::Send::INSTANCE;
    constexpr uint64_t FREE             = Message::Send::FREE;
    constexpr uint64_t CHAT_MGR         = 0x1B8E930;
    constexpr uint64_t PRE_LOCAL_ID_MGR = 0x214D430;
    constexpr uint64_t PRE_DOWNLOAD_MGR = 0x1C17660;
    constexpr uint64_t PUSH_ATTACH_TASK = 0x1CE8500;

    namespace Sns
    {
        constexpr uint64_t DATA_MGR = 0x21F00D0;
        constexpr uint64_t TIMELINE = 0x2DD2320;
        constexpr uint64_t FIRST    = 0x2E40810;
        constexpr uint64_t NEXT     = 0x2E663C0;
    }
}
}
