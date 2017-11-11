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

struct ShaderMacroDesc
{
    std::string name;
    uint32_t value;

    bool operator<(const ShaderMacroDesc& b) const
    {
        return this->value < b.value;
    }

    ShaderMacroDesc()
        : name()
        , value(0)
    {
    }

    ShaderMacroDesc(const std::string& name, uint32_t value)
        : name(name)
        , value(value)
    {
    }
};

using ShaderMacros = std::vector<ShaderMacroDesc>;

struct ShaderDesc
{
    ShaderType type;
    std::string filename;
    std::vector<ShaderMacroDesc> macros;
};

class Shader
{
    friend class Pipeline;

    DevicePtr mDevice;
    VkShaderModule mShaderModule;

    bool CompileToFile(ShaderType type, const std::string& shaderFile, const std::string& spvShaderFile,
                       const ShaderMacros& macros, std::vector<uint32_t>& code);
    bool CreateVkShaderModule(const std::vector<uint32_t>& code);

public:
    Shader();
    ~Shader();

    bool Init(const DevicePtr& device, const ShaderDesc& desc);
};

using ShaderPtr = std::shared_ptr<Shader>;

} // namespace Renderer
} // namespace ABench
