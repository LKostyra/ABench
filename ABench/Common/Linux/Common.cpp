#include "PCH.hpp"
#include "../Common.hpp"
#include "../Logger.hpp"

namespace ABench {
namespace Common {

// This function should be unused under Linux
bool UTF8ToUTF16(const std::string& in, std::wstring& out)
{
    UNUSED(in); UNUSED(out);
    LOGE("Linux has no use for UTF-8 to UTF-16 converter. If you got here, you're doing something wrong.");
    return false;
}

} // namespace ABench
} // namespace Common
