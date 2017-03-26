#pragma once

namespace ABench {
namespace Common {

class Timer
{
    LARGE_INTEGER mFreq;
    LARGE_INTEGER mStart;

public:
    Timer();
    ~Timer();

    void Start(); // Starts the time measurement
    double Stop(); // Stops the measurement, returns time from Start() in seconds
};

} // namespace Common
} // namespace ABench
