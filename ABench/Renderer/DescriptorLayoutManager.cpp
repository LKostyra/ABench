#include "PCH.hpp"
#include "DescriptorLayoutManager.hpp"

#include "Util.hpp"
#include "Extensions.hpp"
#include "Tools.hpp"


namespace ABench {
namespace Renderer {

DescriptorLayoutManager::DescriptorLayoutManager()
    : mDevice(VK_NULL_HANDLE)
    , mFragmentShaderSampler(VK_NULL_HANDLE)
    , mVertexShaderLayout(VK_NULL_HANDLE)
    , mFragmentShaderLayout(VK_NULL_HANDLE)
    , mFragmentShaderDiffuseTextureLayout(VK_NULL_HANDLE)
    , mFragmentShaderNormalTextureLayout(VK_NULL_HANDLE)
    , mFragmentShaderMaskTextureLayout(VK_NULL_HANDLE)
    , mAllShaderLayout(VK_NULL_HANDLE)
{
}

DescriptorLayoutManager::~DescriptorLayoutManager()
{
    Release();
}

DescriptorLayoutManager& DescriptorLayoutManager::Instance()
{
    static DescriptorLayoutManager instance;
    return instance;
}

VkSampler DescriptorLayoutManager::CreateSampler()
{
    VkSampler sampler;

    VkSamplerCreateInfo sampInfo;
    ZERO_MEMORY(sampInfo);
    sampInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampInfo.magFilter = VK_FILTER_LINEAR;
    sampInfo.minFilter = VK_FILTER_LINEAR;
    sampInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampInfo.mipLodBias = 0.0f;
    sampInfo.anisotropyEnable = VK_FALSE;
    sampInfo.maxAnisotropy = 1.0f;
    sampInfo.compareEnable = VK_FALSE;
    sampInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    sampInfo.minLod = FLT_MIN;
    sampInfo.maxLod = FLT_MAX;
    sampInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    sampInfo.unnormalizedCoordinates = VK_FALSE;
    VkResult result = vkCreateSampler(mDevice, &sampInfo, nullptr, &sampler);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Sampler");

    return sampler;
}

void DescriptorLayoutManager::DestroyLayout(VkDescriptorSetLayout& layout)
{
    if (layout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(mDevice, layout, nullptr);
        layout = VK_NULL_HANDLE;
    }
}

bool DescriptorLayoutManager::Init(const VkDevice device)
{
    mDevice = device;

    // sampler for layouts below
    mFragmentShaderSampler = CreateSampler();
    if (mFragmentShaderSampler == VK_NULL_HANDLE)
        return false;


    // shader resources initialization
    std::vector<DescriptorSetLayoutDesc> vsLayoutDesc;
    vsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, VK_NULL_HANDLE});
    vsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_NULL_HANDLE});
    mVertexShaderLayout = Tools::CreateDescriptorSetLayout(vsLayoutDesc);
    if (mVertexShaderLayout == VK_NULL_HANDLE)
        return false;

    std::vector<DescriptorSetLayoutDesc> fsLayoutDesc;
    fsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE});
    mFragmentShaderLayout = Tools::CreateDescriptorSetLayout(fsLayoutDesc);
    if (mFragmentShaderLayout == VK_NULL_HANDLE)
        return false;

    std::vector<DescriptorSetLayoutDesc> fsTexLayoutDesc;
    fsTexLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, mFragmentShaderSampler});
    mFragmentShaderDiffuseTextureLayout = Tools::CreateDescriptorSetLayout(fsTexLayoutDesc);
    if (mFragmentShaderDiffuseTextureLayout == VK_NULL_HANDLE)
        return false;

    mFragmentShaderNormalTextureLayout = Tools::CreateDescriptorSetLayout(fsTexLayoutDesc);
    if (mFragmentShaderNormalTextureLayout == VK_NULL_HANDLE)
        return false;

    mFragmentShaderMaskTextureLayout = Tools::CreateDescriptorSetLayout(fsTexLayoutDesc);
    if (mFragmentShaderMaskTextureLayout == VK_NULL_HANDLE)
        return false;

    std::vector<DescriptorSetLayoutDesc> allLayoutDesc;
    allLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, VK_NULL_HANDLE});
    mAllShaderLayout = Tools::CreateDescriptorSetLayout(allLayoutDesc);
    if (mAllShaderLayout == VK_NULL_HANDLE)
        return false;

    return true;
}

void DescriptorLayoutManager::Release()
{
    DestroyLayout(mAllShaderLayout);
    DestroyLayout(mFragmentShaderDiffuseTextureLayout);
    DestroyLayout(mFragmentShaderNormalTextureLayout);
    DestroyLayout(mFragmentShaderMaskTextureLayout);
    DestroyLayout(mFragmentShaderLayout);
    DestroyLayout(mVertexShaderLayout);

    if (mFragmentShaderSampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(mDevice, mFragmentShaderSampler, nullptr);
        mFragmentShaderSampler = VK_NULL_HANDLE;
    }
}

} // namespace Renderer
} // namespace ABench
