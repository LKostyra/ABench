#include "../../PCH.hpp"
#include "../Library.hpp"
#include "../Common.hpp"

namespace ABench {
namespace Common {

Library::Library()
{
}

Library::~Library()
{
    Close();
}

bool Library::Open(const std::string& path)
{
    UNUSED(path);
    return false;
}

void* Library::GetSymbol(const std::string& name)
{
    UNUSED(name);
    return nullptr;
}

void Library::Close()
{
}

} // namespace Common
} // namespace ABench
