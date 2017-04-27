#pragma once

namespace ABench {
namespace Common {

class Library
{
#ifdef WIN32
    HMODULE mModule;
#elif defined(__linux__) | defined(__LINUX__)
    void* mModule;
#else
#error "Target platform not supported."
#endif


public:
    Library();
    ~Library();

    /**
     * Opens a library for further symbol loading.
     */
    bool Open(const std::string& libname);
    void* GetSymbol(const std::string& name);
    void Close();
};

} // namespace Common
} // namespace ABench
