#pragma once

namespace ABench {
namespace Common {

class Library
{
    HMODULE mModule;

public:
    Library();
    ~Library();

    bool Open(const std::string& path);
    void* GetSymbol(const std::string& name);
    void Close();
};

} // namespace Common
} // namespace ABench
