#pragma once

namespace ABench {
namespace Common {

//// TypeName trait ////

template <typename T>
class TypeName
{
public:
    static const char* Get()
    {
        return typeid(T).name();
    }
};

} // namespace Common
} // namespace ABench
