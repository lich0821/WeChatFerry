#include "log.h"

#define LOGGER_NAME      "WCF"
#define LOGGER_FILE_NAME "logs/wcf.txt"
#define LOGGER_MAX_SIZE  1024 * 1024 * 10 // 10M
#define LOGGER_MAX_FILES 10               // 10 files

void InitLogger()
{
    static std::shared_ptr<spdlog::logger> gLogger = nullptr;
    if (gLogger != nullptr) {
        return;
    }

    gLogger = spdlog::rotating_logger_mt(LOGGER_NAME, LOGGER_FILE_NAME, LOGGER_MAX_SIZE, LOGGER_MAX_FILES);
    // gLogger = spdlog::stdout_color_mt("console");

    spdlog::set_default_logger(gLogger);
    gLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] [%s::%#::%!] %v");
#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG
    spdlog::set_level(spdlog::level::debug);
    gLogger->flush_on(spdlog::level::debug);
#else
    gLogger->flush_on(spdlog::level::info);
#endif
    LOG_DEBUG("InitLogger with debug level");
}

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG

#define BUF_SIZE (1024 * 1024)
static char buf[BUF_SIZE] = { 0 };

void log_buffer(uint8_t *buffer, size_t len)
{
    size_t j = sprintf_s(buf, BUF_SIZE, "Encoded message[%ld]: ", len);
    for (size_t i = 0; i < len; i++) {
        j += sprintf_s(buf + j, BUF_SIZE, "%02X ", buffer[i]);
        if (j > BUF_SIZE - 3) {
            break;
        }
    }
    LOG_DEBUG(buf);
}
#endif
