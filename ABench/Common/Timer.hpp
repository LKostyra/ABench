#pragma once

namespace ABench {
namespace Common {

class Timer
{
#ifdef WIN32
    LARGE_INTEGER mFreq;
    LARGE_INTEGER mStart;
#elif defined(__linux__) | defined(__LINUX__)
#else
#error "Target platform not supported."
#endif

public:
    Timer();
    ~Timer();

    void Start(); // Starts the time measurement
    double Stop(); // Stops the measurement, returns time from Start() in seconds
};

} // namespace Common
} // namespace ABench
