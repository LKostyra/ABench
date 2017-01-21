#include "../PCH.hpp"
#include "Logger.hpp"

#include <iostream>
#include <fstream>

namespace ABench {
namespace Logger {

std::ofstream logFile;

void Log(LogLevel level, const std::stringstream& msg)
{
    const char* levelStr = nullptr;

    static HANDLE console = 0;
    if (!console)
        console = GetStdHandle(STD_OUTPUT_HANDLE);

    if (!logFile.is_open())
    {
        logFile.open("log.txt");
    }

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

    std::stringstream fullMsg;
    fullMsg << "[" << levelStr << "] " << msg.str() << "\n";
    std::cout << fullMsg.str();
    if (logFile.is_open())
        logFile << fullMsg.str();

    SetConsoleTextAttribute(console, conInfo.wAttributes);
}

} // namespace Logger
} // namespace ABench
