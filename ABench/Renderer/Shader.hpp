#pragma once

#include "Prerequisites.hpp"
#include "Device.hpp"

#include <memory>

namespace ABench {
namespace Renderer {

enum class ShaderType: unsigned char
{
    UNKNOWN = 0,
    VERTEX,
    TESS_CONTROL,
    TESS_EVAL,
    GEOMETRY,
    FRAGMENT,
    COMPUTE
};

struct ShaderDesc
{
    ShaderType type;
    std::string filename;
};

class Shader
{
    friend class Pipeline;

    VkShaderModule mShaderModule;

    bool CompileToFile(ShaderType type, const std::string& filename, std::vector<uint32_t>& code);
    bool CreateVkShaderModule(const std::vector<uint32_t>& code);

public:
    Shader();
    ~Shader();

    bool Init(const ShaderDesc& desc);
};

} // namespace Renderer
} // namespace ABench
