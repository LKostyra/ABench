#include "../PCH.hpp"
#include "Shader.hpp"
#include "Util.hpp"
#include "Extensions.hpp"

#include "Common/Common.hpp"

#include <fstream>


namespace ABench {
namespace Renderer {

Shader::Shader()
    : mDevicePtr(nullptr)
    , mShaderModule(VK_NULL_HANDLE)
{
}

Shader::~Shader()
{
    if (mShaderModule != VK_NULL_HANDLE)
        vkDestroyShaderModule(mDevicePtr->GetDevice(), mShaderModule, nullptr);
}

bool Shader::Compile(ShaderLanguage lang, const std::string& path, std::unique_ptr<uint32_t[]>& code, size_t& codeSize)
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
    mDevicePtr = desc.devicePtr;

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
        std::ifstream codeFile(desc.path, std::ifstream::in | std::ifstream::binary);
        if (!codeFile.good())
        {
            LOGE("Failed to open Shader code file " << desc.path);
            return false;
        }

        codeFile.seekg(0, std::ifstream::end);
        codeSize = static_cast<size_t>(codeFile.tellg());
        codeFile.seekg(0, std::ifstream::beg);

        code.reset(new uint32_t [codeSize/sizeof(uint32_t)]);
        codeFile.read(reinterpret_cast<char*>(code.get()), codeSize);
        if (!codeFile.good())
        {
            LOGE("Failure while reading the file");
            LOGE("bits: " << codeFile.eof() << codeFile.fail() << codeFile.bad());
        }
    }

    VkShaderModuleCreateInfo shaderInfo;
    ZERO_MEMORY(shaderInfo);
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.pCode = code.get();
    shaderInfo.codeSize = codeSize;
    VkResult result = vkCreateShaderModule(mDevicePtr->GetDevice(), &shaderInfo, nullptr, &mShaderModule);
    RETURN_FALSE_IF_FAILED(result, "Failed to create shader module from file " << desc.path);

    LOGI("Successfully initialized shader module from " << desc.path);
    return true;
}

} // namespace Renderer
} // namespace ABench
