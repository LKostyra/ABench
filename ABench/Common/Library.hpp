#pragma once

namespace ABench {
namespace Common {

class Library
{
#ifdef WIN32
    HMODULE mModule;
#elif defined(__linux__) | defined(__LINUX__)
#else
#error "Target platform not supported."
#endif


public:
    Library();
    ~Library();

    bool Open(const std::string& path);
    void* GetSymbol(const std::string& name);
    void Close();
};

} // namespace Common
} // namespace ABench
