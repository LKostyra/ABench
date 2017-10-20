#pragma once

#include <cstring>
#include "Common/Logger.hpp"

// Common defines
#define ZERO_MEMORY(x) memset(&x, 0, sizeof(x))
#define UNUSED(x) (void)(x)

#ifdef WIN32
#define ABENCH_INLINE __forceinline
#elif defined(__linux__) | defined(__LINUX__)
#define ABENCH_INLINE __attribute__((always_inline))
#else
#error "Target platform not supported"
#endif

#ifdef _DEBUG
#define ASSERT(exp, msg) do \
{ \
    if (!(exp)) \
    { \
        LOGE("Assertion " << #exp << " failed: " << msg); \
        assert(exp); \
    } \
} while(0)
#else // _DEBUG
#define ASSERT(exp, msg) do { } while(0)
#endif // _DEBUG

namespace ABench {
namespace Common {

bool UTF8ToUTF16(const std::string& in, std::wstring& out);
bool UTF16ToUTF8(const std::wstring& in, std::string& out);

} // namespace Common
} // namespace ABench
