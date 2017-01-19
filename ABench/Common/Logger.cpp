#include "../PCH.hpp"
#include "Logger.hpp"

#include <iostream>


namespace ABench {
namespace Logger {

void Log(LogLevel level, const std::stringstream& msg)
{
    const char* levelStr = nullptr;

    static HANDLE console = 0;
    if (!console)
        console = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO conInfo;
    GetConsoleScreenBufferInfo(console, &conInfo);

    switch (level)
    {
    case LogLevel::DEBUG:
        levelStr = "DEBUG";
        SetConsoleTextAttribute(console, FOREGROUND_BLUE);
        break;
    case LogLevel::INFO:
        levelStr = " INF ";
        break;
    case LogLevel::WARNING:
        levelStr = " WRN ";
        SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN);
        break;
    case LogLevel::ERROR:
        levelStr = "ERROR";
        SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_INTENSITY);
        break;
    }

    std::cout << "[" << levelStr << "] " << msg.str() << std::endl;
    SetConsoleTextAttribute(console, conInfo.wAttributes);
}

} // namespace Logger
} // namespace ABench
