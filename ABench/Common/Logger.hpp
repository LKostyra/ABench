#pragma once

#include <sstream>

#ifdef ERROR
#undef ERROR
#endif

namespace ABench {
namespace Logger {

enum class LogLevel: unsigned char
{
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
};

void Log(LogLevel level, const std::stringstream& msg);

} // namespace Logger
} // namespace ABench

#define LOG(level, msg) do { \
    std::stringstream ss; \
    ss << msg; \
    ABench::Logger::Log(level, ss); \
} while(0)

#define LOGD(msg) LOG(ABench::Logger::LogLevel::DEBUG, msg)
#define LOGI(msg) LOG(ABench::Logger::LogLevel::INFO, msg)
#define LOGW(msg) LOG(ABench::Logger::LogLevel::WARNING, msg)
#define LOGE(msg) LOG(ABench::Logger::LogLevel::ERROR, msg)
