#pragma once

#include "framework.h"
#include <string>

typedef uint64_t QWORD;

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
