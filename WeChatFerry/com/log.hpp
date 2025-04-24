#pragma once

#ifdef ENABLE_DEBUG_LOG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#endif

#include <filesystem>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include "framework.h"

#define LOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define LOG_INFO(...)  SPDLOG_INFO(__VA_ARGS__)
#define LOG_WARN(...)  SPDLOG_WARN(__VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)

#ifdef ENABLE_DEBUG_LOG
#define LOG_BUFFER(buf, len) Log::log_buffer((buf), (len))
#else
#define LOG_BUFFER(...) (void)0
#endif

namespace Log
{
inline constexpr char DEFAULT_LOGGER_NAME[]      = "WCF";
inline constexpr char DEFAULT_LOGGER_FILE[]      = "logs/wcf.txt";
inline constexpr size_t DEFAULT_LOGGER_MAX_SIZE  = 1024 * 1024 * 10; // 10MB
inline constexpr size_t DEFAULT_LOGGER_MAX_FILES = 10;

inline void InitLogger(const std::string &path)
{
    static std::shared_ptr<spdlog::logger> logger = nullptr;

    if (logger != nullptr) {
        return; // 已初始化
    }

    std::filesystem::path filename = std::filesystem::path(path) / DEFAULT_LOGGER_FILE;
    std::filesystem::path logDir   = filename.parent_path();

    if (!std::filesystem::exists(logDir)) {
        std::filesystem::create_directories(logDir);
    }

    try {
        logger = spdlog::rotating_logger_mt(DEFAULT_LOGGER_NAME, filename.string(), DEFAULT_LOGGER_MAX_SIZE,
                                            DEFAULT_LOGGER_MAX_FILES);
    } catch (const spdlog::spdlog_ex &ex) {
        MessageBoxA(NULL, ex.what(), "Init LOGGER ERROR", MB_ICONERROR);
        return;
    }

    spdlog::set_default_logger(logger);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] [%s::%#::%!] %v");

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG
    spdlog::set_level(spdlog::level::debug);
    logger->flush_on(spdlog::level::debug);
#else
    logger->flush_on(spdlog::level::info);
#endif

    LOG_DEBUG("InitLogger with debug level");
}

#ifdef ENABLE_DEBUG_LOG
inline void log_buffer(uint8_t *buffer, size_t len)
{
    constexpr size_t BUF_SIZE = 1024 * 1024;
    std::ostringstream oss;

    oss << "BUF@" << static_cast<void *>(buffer) << "[" << len << "]: ";
    for (size_t i = 0; i < len; ++i) {
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]) << " ";
        if (oss.tellp() > BUF_SIZE - 3) {
            break; // 防止缓冲区溢出
        }
    }

    LOG_DEBUG(oss.str());
}
#endif

} // namespace Log
