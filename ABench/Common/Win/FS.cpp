#include "PCH.hpp"
#include "../FS.hpp"

#include "Common/Common.hpp"
#include "Common/Logger.hpp"

namespace ABench {
namespace Common {
namespace FS {

std::string GetExecutablePath()
{
    HMODULE exe = GetModuleHandle(0);

    WCHAR path[512];
    GetModuleFileName(exe, path, 512);

    std::wstring pathWStr(path);
    std::string pathStr;
    ABench::Common::UTF16ToUTF8(pathWStr, pathStr);

    std::string unifiedPathStr = UnifySlashes(pathStr);
    return unifiedPathStr;
}

std::string GetParentDir(const std::string& path)
{
    size_t trailingSlash = path.rfind('/');
    return path.substr(0, trailingSlash);
}

std::string GetFileName(const std::string& path)
{
    size_t trailingSlash = path.rfind('/');
    if (trailingSlash == std::string::npos)
        return path;

    return path.substr(trailingSlash + 1);
}

std::string JoinPaths(const std::string& a, const std::string& b)
{
    std::string first, second;

    if (a.back() == '/')
        first = a.substr(0, a.size() - 2);
    else
        first = a;

    if (b.front() == '/')
        second = b.substr(1);
    else
        second = b;

    return first + '/' + second;
}

bool SetCWD(const std::string& path)
{
    std::wstring wPath;
    ABench::Common::UTF8ToUTF16(path, wPath);
    if (!SetCurrentDirectory(wPath.c_str()))
    {
        DWORD err = GetLastError();
        LOGE("Failed to set current working directory: " << err);
        return false;
    }

    WCHAR cwdBuf[512];
    GetCurrentDirectory(512, cwdBuf);

    std::wstring cwdWStr(cwdBuf);
    std::string cwdStr;
    UTF16ToUTF8(cwdWStr, cwdStr);

    LOGI("Set current working directory to: " << UnifySlashes(cwdStr));
    return true;
}

std::string UnifySlashes(const std::string& path)
{
    std::string result = path;

    for (char& c : result)
    {
        if (c == '\\')
            c = '/';
    }

    return result;
}

} // namespace FS
} // namespace Common
} // namespace ABench
