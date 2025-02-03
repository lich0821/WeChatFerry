#pragma once

#include "framework.h"
#include <string>

typedef uint64_t QWORD;

class WxString
{

public:
    const wchar_t *wptr;
    DWORD size;
    DWORD capacity;
    const char *ptr;
    DWORD clen;

    WxString()
    {
        wptr     = nullptr;
        size     = 0;
        capacity = 0;
        ptr      = nullptr;
        clen     = 0;
    }

    WxString(std::wstring ws) : internal_ws(std::move(ws))
    {
        wptr     = internal_ws.c_str();
        size     = static_cast<DWORD>(internal_ws.size());
        capacity = static_cast<DWORD>(internal_ws.capacity());
        ptr      = nullptr;
        clen     = 0;
    }

    WxString(const WxString &)            = delete;
    WxString &operator=(const WxString &) = delete;

    WxString(WxString &&) noexcept            = default;
    WxString &operator=(WxString &&) noexcept = default;

private:
    std::wstring internal_ws;
};

typedef struct RawVector {
#ifdef _DEBUG
    QWORD head;
#endif
    QWORD start;
    QWORD finish;
    QWORD end;
} RawVector_t;
