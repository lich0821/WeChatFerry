#ifndef OFFSETS_H
#define OFFSETS_H

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
    }
}
}

#endif // OFFSETS_H
