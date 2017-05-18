#include "PCH.hpp"
#include "../Timer.hpp"


namespace ABench {
namespace Common {

namespace {

const time_t NS_IN_SECOND = 1'000'000'000;

} // namespace

Timer::Timer()
{
    mStart.tv_sec = 0;
    mStart.tv_nsec = 0;
}

Timer::~Timer()
{
}

void Timer::Start()
{
    clock_gettime(CLOCK_MONOTONIC, &mStart);
}

double Timer::Stop()
{
    timespec stop;
    clock_gettime(CLOCK_MONOTONIC, &stop);

    time_t resultSeconds = (stop.tv_sec - mStart.tv_sec) * NS_IN_SECOND;
    return static_cast<double>(resultSeconds + (stop.tv_nsec - mStart.tv_nsec)) * 1e-9;
}

} // namespace Common
} // namespace ABench
