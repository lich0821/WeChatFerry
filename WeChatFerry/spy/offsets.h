#ifndef OFFSETS_H
#define OFFSETS_H

#include <cstdint>

namespace Offsets
{

namespace Account
{
    constexpr uint64_t SERVICE = 0x1B58B50; // 账户服务
    constexpr uint64_t PATH    = 0x2250920; // 数据路径
    constexpr uint64_t WXID    = 0x80;      // WXID
    constexpr uint64_t NAME    = 0x1E8;     // 昵称
    constexpr uint64_t MOBILE  = 0x128;     // 手机号
    constexpr uint64_t LOGIN   = 0x7F8;     // 登录状态
}

namespace Message
{
    namespace Log
    {
        constexpr uint64_t FUNCTION = 0x261B890; // 日志函数
        constexpr uint64_t LEVEL    = 0x56E4244; // 日志级别
    }
}
}

#endif // OFFSETS_H
