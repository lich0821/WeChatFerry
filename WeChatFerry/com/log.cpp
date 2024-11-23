#include <filesystem>

#include "log.h"
#include "util.h"

#define LOGGER_NAME      "WCF"
#define LOGGER_FILE_NAME "/logs/wcf.txt"
#define LOGGER_MAX_SIZE  1024 * 1024 * 10 // 10M
#define LOGGER_MAX_FILES 10               // 10 files

void InitLogger(std::string path)
{
    static std::shared_ptr<spdlog::logger> logger = nullptr;
    if (logger != nullptr) {
        return;
    }
    // check and create logs folder
    std::filesystem::path logDir = std::filesystem::path(path) / "logs";
    if (!std::filesystem::exists(logDir)) {
        std::filesystem::create_directory(logDir);
    }
    auto filename = std::filesystem::path(path + LOGGER_FILE_NAME).make_preferred().string();
    try {
        logger = spdlog::rotating_logger_mt(LOGGER_NAME, filename, LOGGER_MAX_SIZE, LOGGER_MAX_FILES);
    } catch (const spdlog::spdlog_ex &ex) {
        MessageBox(NULL, String2Wstring(ex.what()).c_str(), L"Init LOGGER ERROR", 0);
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

void log_buffer(uint8_t *buffer, size_t len)
{
    size_t j = sprintf_s(buf, BUF_SIZE, "BUF@%p[%zd]: ", buffer, len);
    for (size_t i = 0; i < len; i++) {
        j += sprintf_s(buf + j, BUF_SIZE, "%02X ", buffer[i]);
        if (j > BUF_SIZE - 3) {
            break;
        }
    }
    LOG_DEBUG(buf);
}
#endif
