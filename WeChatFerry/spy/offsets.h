#pragma once

#include <cstdint>

namespace Offsets
{

namespace Account
{
    constexpr uint64_t SERVICE = 0x1B58B50; // 账户服务
    constexpr uint64_t PATH    = 0x25E9090; // 数据路径
    constexpr uint64_t WXID    = 0x80;      // WXID
    constexpr uint64_t NAME    = 0x1E8;     // 昵称
    constexpr uint64_t MOBILE  = 0x128;     // 手机号
    constexpr uint64_t LOGIN   = 0x7F8;     // 登录状态
}

namespace Chatroom
{
    constexpr uint64_t MGR = 0x1B86F60;
    constexpr uint64_t DEL = 0x2158830;
    constexpr uint64_t ADD = 0x21581F0;
    constexpr uint64_t INV = 0x2157BD0;
}

namespace Contact
{
    constexpr uint64_t MGR     = 0x1B44B20;
    constexpr uint64_t LIST    = 0x21A1E00;
    constexpr uint64_t BIN     = 0x200;
    constexpr uint64_t BIN_LEN = 0x208;
    constexpr uint64_t WXID    = 0x10;
    constexpr uint64_t CODE    = 0x30;
    constexpr uint64_t REMARK  = 0x80;
    constexpr uint64_t NAME    = 0xA0;
    constexpr uint64_t GENDER  = 0x0E;
    constexpr uint64_t STEP    = 0x6A8;

    constexpr uint64_t VERIFY_NEW        = 0x2621B00;
    constexpr uint64_t VERIFY_OK         = 0x1F421E0;
    constexpr uint64_t VERIFY_MGR        = 0x4F022A8;
    constexpr uint64_t VERIFY_A8         = 0x2621B91;
    constexpr uint64_t ADD_FRIEND_HELPER = 0x4EE4A20;
    constexpr uint64_t FVDF              = 0x4F02768; // FriendVeriyDialogFragment
}

namespace Db
{
    constexpr uint64_t INSTANCE     = 0x59226C8; // 数据库实例地址
    constexpr uint64_t MSG_I        = 0x5980420; // MSGi.db & MediaMsgi.db
    constexpr uint64_t MICROMSG     = 0xB8;
    constexpr uint64_t CHAT_MSG     = 0x2C8;
    constexpr uint64_t MISC         = 0x5F0;
    constexpr uint64_t EMOTION      = 0x15F0;
    constexpr uint64_t MEDIA        = 0xF48;
    constexpr uint64_t BIZCHAT_MSG  = 0x1AC0;
    constexpr uint64_t FUNCTION_MSG = 0x1B98;
    constexpr uint64_t NAME         = 0x28;

    // SQLITE3
    constexpr uint64_t EXEC = 0x3A76430;
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
        constexpr uint64_t LEVEL = 0x56E4244; // 日志级别
        constexpr uint64_t CALL  = 0x261B890; // 日志函数
    }

    namespace Receive
    {
        constexpr uint64_t CALL      = 0x2141E80; // 接收消息 Call
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

        constexpr uint64_t PYQ_CALL    = 0x2E56080; // 接收朋友圈 Call
        constexpr uint64_t PYQ_START   = 0x30;      // 开始地址
        constexpr uint64_t PYQ_END     = 0x38;      // 结束地址
        constexpr uint64_t PYQ_SENDER  = 0x18;      // 发布者
        constexpr uint64_t PYQ_TS      = 0x38;      // 时间戳
        constexpr uint64_t PYQ_CONTENT = 0x48;      // 文本内容
        constexpr uint64_t PYQ_XML     = 0x9B8;     // 其他内容
    }

    namespace Send
    {
        constexpr uint64_t MGR          = 0x1B57350;
        constexpr uint64_t INSTANCE     = 0x1B614C0;
        constexpr uint64_t FREE         = 0x1B58BD0;
        constexpr uint64_t TEXT         = 0x22C9CA0;
        constexpr uint64_t IMAGE        = 0x22BF430;
        constexpr uint64_t APP_MGR      = 0x1B5C2F0;
        constexpr uint64_t FILE         = 0x20D30E0;
        constexpr uint64_t XML          = 0x20D2210;
        constexpr uint64_t XML_BUF_SIGN = 0x24F95C0;
        constexpr uint64_t EMOTION_MGR  = 0x1BD2310;
        constexpr uint64_t EMOTION      = 0x21B8100;

        constexpr uint64_t NEW_MM_READER  = 0x1B60A10;
        constexpr uint64_t FREE_MM_READER = 0x1B5FDE0;
        constexpr uint64_t RICH_TEXT      = 0x20DD0C0;

        constexpr uint64_t PAT = 0x2CC1E90;

        constexpr uint64_t FORWARD = 0x22C9220;
    }
}

namespace Misc
{
    constexpr uint64_t QR_CODE = 0x2025A80;

    constexpr uint64_t INSATNCE         = Message::Send::INSTANCE;
    constexpr uint64_t FREE             = Message::Send::FREE;
    constexpr uint64_t CHAT_MGR         = 0x1B8AA50;
    constexpr uint64_t PRE_LOCAL_ID_MGR = 0x2142BF0;
    constexpr uint64_t PRE_DOWNLOAD_MGR = 0x1C12260;
    constexpr uint64_t PUSH_ATTACH_TASK = 0x1CE3050;

    namespace Sns
    {
        constexpr uint64_t DATA_MGR = 0x21E52F0;
        constexpr uint64_t TIMELINE = 0x2DC6180;
        constexpr uint64_t FIRST    = 0x2E346C0;
        constexpr uint64_t NEXT     = 0x2E5A270;
    }
}
}
