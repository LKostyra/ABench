#pragma once

#include "Common/Common.hpp"


namespace ABench {
namespace Scene {

enum class ComponentType: unsigned char
{
    Invalid = 0,
    Model,
    Light,
    Emitter,
};

class Component
{
protected:
    std::string mName;

public:
    Component(const std::string& name);

    virtual ComponentType GetType() const = 0;

    ABENCH_INLINE std::string GetName() const
    {
        return mName;
    }
};

} // namespace Scene
} // namespace ABench
