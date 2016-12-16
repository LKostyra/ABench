#pragma once

#include <cstring>

#define ZERO_MEMORY(x) memset(&x, 0, sizeof(x))
#define UNUSED(x) (void)(x)

namespace ABench {
namespace Common {

bool UTF8ToUTF16(const std::string& in, std::wstring& out);

} // namespace Common
} // namespace ABench
