#pragma once

#include "Prerequisites.hpp"
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

    VkShaderModule mShaderModule;

    bool Compile(ShaderLanguage lang, const std::string& path, std::unique_ptr<uint32_t[]>& code, size_t& codeSize);

public:
    Shader();
    ~Shader();

    bool Init(const ShaderDesc& desc);
};

} // namespace Renderer
} // namespace ABench
