#pragma once

namespace ABench {
namespace Scene {

enum class ComponentType: unsigned char
{
    Invalid = 0,
    Mesh,
    Light,
};

class Component
{
public:
    virtual ComponentType GetType() const = 0;
};

} // namespace Scene
} // namespace ABench
