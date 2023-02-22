#include "log.h"

#define LOGGER_NAME      "WCF"
#define LOGGER_FILE_NAME "logs/wcf.txt"
#define LOGGER_MAX_SIZE  1024 * 1024 * 10 // 10M
#define LOGGER_MAX_FILES 10               // 10 files

void InitLogger()
{
#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif
    static std::shared_ptr<spdlog::logger> gLogger = nullptr;
    if (gLogger != nullptr) {
        return;
    }

    gLogger = spdlog::rotating_logger_mt(LOGGER_NAME, LOGGER_FILE_NAME, LOGGER_MAX_SIZE, LOGGER_MAX_FILES);
    // gLogger = spdlog::stdout_color_mt("console");

    spdlog::set_default_logger(gLogger);
    gLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] [%s::%#::%!] %v");
    gLogger->flush_on(spdlog::level::info);

    LOG_DEBUG("InitLogger with debug level");
}
