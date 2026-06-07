#pragma once

#include "framework.h"
#include <string>

// 注：所有内部地址/字段偏移都统一收敛到 offsets.h 的命名空间
// （Message::Receive/Send、Moments、Database、Chatroom 等），并以带类型的函数指针建模调用。

// 微信内部字符串对象视图：wptr 指向宽字符缓冲（通常是本进程 std::wstring::c_str()），
// 为非拥有视图；ptr/clen 为可选窄字符侧，默认空。
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
