#pragma once

#include <sstream>

#ifdef ERROR
#undef ERROR
#endif

namespace ABench {
namespace Logger {

enum class LogLevel: unsigned char
{
    ERROR = 0,
    WARNING,
    INFO,
    DEBUG,
};

void Log(LogLevel level, const std::stringstream& msg);

} // namespace Logger
} // namespace ABench

#define LOG(level, msg) do { \
    std::stringstream ss; \
    ss << msg; \
    ABench::Logger::Log(level, ss); \
} while(0)

#ifdef _DEBUG
#define LOGD(msg) LOG(ABench::Logger::LogLevel::DEBUG, msg)
#else
#define LOGD(msg) do { } while(0)
#endif

#define LOGI(msg) LOG(ABench::Logger::LogLevel::INFO, msg)
#define LOGW(msg) LOG(ABench::Logger::LogLevel::WARNING, msg)
#define LOGE(msg) LOG(ABench::Logger::LogLevel::ERROR, msg)
