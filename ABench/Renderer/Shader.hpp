#pragma once

#include "Device.hpp"

#include <memory>

namespace ABench {
namespace Renderer {

enum class ShaderLanguage: unsigned char
{
    SPIRV = 0,
    GLSL,
};

struct ShaderDesc
{
    ShaderLanguage language;
    std::string path;
};

class Shader
{
    friend class Pipeline;

    const Device* mDevicePtr;

    VkShaderModule mShaderModule;

    bool Compile(ShaderLanguage lang, const std::string& path, std::unique_ptr<uint32_t[]>& code, size_t& codeSize);

public:
    Shader(const Device* device);
    ~Shader();

    bool Init(const ShaderDesc& desc);
};

} // namespace Renderer
} // namespace ABench
