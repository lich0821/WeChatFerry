#pragma once

#include <filesystem>
#include <memory>
#include <string>

#ifdef ENABLE_DEBUG_LOG
#include <cstdint>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
void log_buffer(uint8_t *buffer, size_t len);
#define LOG_BUFFER(buf, len) log_buffer((buf), (len))
#else
#define LOG_BUFFER(...) (void)0
#endif

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#define LOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__);
#define LOG_INFO(...)  SPDLOG_INFO(__VA_ARGS__);
#define LOG_WARN(...)  SPDLOG_WARN(__VA_ARGS__);
#define LOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__);

#define LOGGER_NAME      "WCF"
#define LOGGER_FILE_NAME "/logs/wcf.txt"
#define LOGGER_MAX_SIZE  1024 * 1024 * 10 // 10M
#define LOGGER_MAX_FILES 10               // 10 files

inline void init_logger(const std::string &path)
{
    static std::shared_ptr<spdlog::logger> logger = nullptr;
    if (logger != nullptr) {
        return;
    }

    auto filename = std::filesystem::path(path + LOGGER_FILE_NAME).make_preferred().string();
    try {
        logger = spdlog::rotating_logger_mt(LOGGER_NAME, filename, LOGGER_MAX_SIZE, LOGGER_MAX_FILES);
    } catch (const spdlog::spdlog_ex &ex) {
        std::wstring wmsg(ex.what(), ex.what() + strlen(ex.what()));
        MessageBox(NULL, wmsg.c_str(), L"Init LOGGER ERROR", 0);
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

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG

#define BUF_SIZE (1024 * 1024)
static char buf[BUF_SIZE] = { 0 };

inline void log_buffer(uint8_t *buffer, size_t len)
{
    size_t j = sprintf_s(buf, BUF_SIZE, "BUF@%08X[%ld]: ", (uint32_t)buffer, len);
    for (size_t i = 0; i < len; i++) {
        j += sprintf_s(buf + j, BUF_SIZE, "%02X ", buffer[i]);
        if (j > BUF_SIZE - 3) {
            break;
        }
    }
    LOG_DEBUG(buf);
}
#endif
