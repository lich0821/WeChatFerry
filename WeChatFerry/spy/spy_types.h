#pragma once

#include "framework.h"
#include <string>

typedef uint64_t QWORD;

class WxString
{
public:
    const wchar_t *wptr;
    DWORD size;
    DWORD length;
    const char *ptr;
    DWORD clen;

    WxString() : wptr(nullptr), size(0), length(0), ptr(nullptr), clen(0) { }

    explicit WxString(const std::wstring &ws)
        : wptr(ws.c_str()), size(static_cast<DWORD>(ws.size())), length(static_cast<DWORD>(ws.length())), ptr(nullptr),
          clen(0)
    {
    }

    WxString(const WxString &)            = delete;
    WxString &operator=(const WxString &) = delete;

    WxString(WxString &&other) noexcept
        : wptr(other.wptr), size(other.size), length(other.length), ptr(other.ptr), clen(other.clen)
    {
        other.wptr   = nullptr;
        other.size   = 0;
        other.length = 0;
        other.ptr    = nullptr;
        other.clen   = 0;
    }

    WxString &operator=(WxString &&other) noexcept
    {
        if (this != &other) {
            wptr   = other.wptr;
            size   = other.size;
            length = other.length;
            ptr    = other.ptr;
            clen   = other.clen;

            other.wptr   = nullptr;
            other.size   = 0;
            other.length = 0;
            other.ptr    = nullptr;
            other.clen   = 0;
        }
        return *this;
    }
};
