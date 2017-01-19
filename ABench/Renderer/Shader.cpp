#include "../PCH.hpp"
#include "Shader.hpp"
#include "Util.hpp"
#include "Extensions.hpp"

#include "Common/Common.hpp"

#include <fstream>


namespace ABench {
namespace Renderer {

Shader::Shader(const Device* device)
    : mDevicePtr(device)
    , mShaderModule(VK_NULL_HANDLE)
{
}

Shader::~Shader()
{
    if (mShaderModule != VK_NULL_HANDLE)
        vkDestroyShaderModule(mDevicePtr->mDevice, mShaderModule, nullptr);
}

bool Shader::Compile(ShaderLanguage lang, const std::string& path, std::unique_ptr<uint32_t>& code, size_t codeSize)
{
#ifndef _DEBUG
    // a little warning to not forget before release
    LOGW("Shaders should not be compiled, rather taken in precompiled SPIRV form!");
#endif

    // TODO
    return false;
}

bool Shader::Init(const ShaderDesc& desc)
{
    std::unique_ptr<uint32_t[]> code;
    size_t codeSize;

    if (desc.language != ShaderLanguage::SPIRV)
    {
        if (!Compile(desc.language, desc.path, code, codeSize))
        {
            LOGE("Failed to compile shader to SPIRV");
            return false;
        }
    }
    else
    {
        std::ifstream codeFile(desc.path);
        if (!codeFile.good())
        {
            LOGE("Failed to open Shader code from file " << desc.path);
            return false;
        }

        codeFile.seekg(0, std::ifstream::end);
        codeSize = codeFile.tellg();
        codeFile.seekg(0, std::ifstream::beg);

        code.reset(new uint32_t [codeSize]);
        codeFile.read(reinterpret_cast<char*>(code.get()), codeSize);
    }

    VkShaderModuleCreateInfo shaderInfo;
    ZERO_MEMORY(shaderInfo);
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.pCode = code.get();
    shaderInfo.codeSize = codeSize;
    VkResult result = vkCreateShaderModule(mDevicePtr->mDevice, &shaderInfo, nullptr, &mShaderModule);
    CHECK_VKRESULT(result, "Failed to create shader module from file " << desc.path);

    LOGI("Successfully initialized shader module from " << desc.path);
    return true;
}

} // namespace Renderer
} // namespace ABench
