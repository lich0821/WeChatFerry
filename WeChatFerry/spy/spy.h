#pragma once

#include <atomic>
#include <cstdint>
#include <string_view>

namespace Spy
{
constexpr std::string_view SUPPORT_VERSION = "3.9.12.51";
inline std::atomic<std::uintptr_t> WeChatDll { 0 };

template <typename T> inline T getFunction(std::uintptr_t offset) { return reinterpret_cast<T>(WeChatDll + offset); }
template <typename T> inline T getFunction(std::uintptr_t base, std::uintptr_t offset)
{
    return reinterpret_cast<T>(base + offset);
}

int Init(void *args);
void Cleanup();
}
