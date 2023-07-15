#pragma once

#include <string>

#ifdef ENABLE_DEBUG_LOG
#include <stdint.h>

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

void InitLogger(std::string path);
