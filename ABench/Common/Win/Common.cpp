#include "PCH.hpp"
#include "../Common.hpp"
#include "../Logger.hpp"

#include <strsafe.h>

namespace ABench {
namespace Common {

bool UTF8ToUTF16(const std::string& in, std::wstring& out)
{
    const int bufSize = 1024;
    wchar_t buf[bufSize];

    // check how many chars we need in UTF16
    size_t inChars;
    HRESULT hr = StringCchLengthA(in.c_str(), INT_MAX - 1, &inChars);
    if (FAILED(hr))
    {
        DWORD err = GetLastError();
        LOGE("Acquiring UTF16 string length failed with error " << err);
        return false;
    }

    ++inChars; // trailing zero

    int result = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, in.c_str(),
                                     static_cast<int>(inChars), buf, bufSize);
    if (!result)
    {
        DWORD err = GetLastError();
        LOGE("UTF8 to UTF16 conversion failed with error " << err);
        return false;
    }

    out = buf;
    return true;
}

bool UTF16ToUTF8(const std::wstring& in, std::string& out)
{
    const int bufferSize = 1024;
    char buffer[bufferSize];

    size_t inChars;
    HRESULT hr = ::StringCchLengthW(in.c_str(), INT_MAX - 1, &inChars);
    if (FAILED(hr))
        return false;

    ++inChars;

    int result = ::WideCharToMultiByte(CP_UTF8, 0, in.c_str(), static_cast<int>(inChars), buffer,
                                       bufferSize, 0, 0);
    if (!result)
    {
        DWORD err = GetLastError();
        LOGE("UTF16 to UTF8 conversion failed with error " << err);
        return false;
    }

    out = buffer;
    return true;
}

} // namespace ABench
} // namespace Common
