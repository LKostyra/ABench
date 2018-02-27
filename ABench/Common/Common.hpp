#pragma once

#include <cstring>
#include "Common/Logger.hpp"

// Common defines
#define ZERO_MEMORY(x) memset(&x, 0, sizeof(x))
#define UNUSED(x) (void)(x)

#ifdef WIN32

#define ABENCH_INLINE __forceinline
#define ABENCH_ALIGN(x) __declspec(align(x))
#define ABENCH_RETURN_ALIGNED_MALLOC(x, o) do { \
    return _aligned_malloc(x, o); \
} while(0)
#define ABENCH_ALIGNED_FREE(x) _aligned_free(x);

#elif defined(__linux__) || defined(__LINUX__)

#define ABENCH_INLINE __attribute__((always_inline))
#define ABENCH_ALIGN(x) __attribute__((aligned(bytes)))
#define ABENCH_RETURN_ALIGNED_MALLOC(x, o) do { \
    o = std::max(o, sizeof(void*)); \
    void* ptr = nullptr; \
    int ret = posix_memalign(&ptr, o, x); \
    if (ret != 0) \
        return nullptr; \
    return ptr; \
} while(0)
#define ABENCH_ALIGNED_FREE(x) free(x)

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
