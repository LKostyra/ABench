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

VkDescriptorSetLayout DescriptorLayoutManager::CreateLayout(std::vector<DescriptorSetLayoutDesc>& descriptors)
{
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;

    std::vector<VkDescriptorSetLayoutBinding> bindings;

    for (uint32_t i = 0; i < descriptors.size(); ++i)
    {
        VkDescriptorSetLayoutBinding binding;
        ZERO_MEMORY(binding);
        binding.descriptorCount = 1;
        binding.binding = i;
        binding.descriptorType = descriptors[i].type;
        binding.stageFlags = descriptors[i].stage;
        if (descriptors[i].sampler != VK_NULL_HANDLE)
            binding.pImmutableSamplers = &descriptors[i].sampler;

        bindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = static_cast<uint32_t>(bindings.size());
    info.pBindings = bindings.data();
    VkResult result = vkCreateDescriptorSetLayout(mDevice, &info, nullptr, &layout);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Descriptor Set Layout");

    LOGD("Created Descriptor Set Layout 0x" << std::hex << reinterpret_cast<size_t*>(layout) <<
         " with " << std::dec << bindings.size() << " bindings.");

    return layout;
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
    mVertexShaderLayout = CreateLayout(vsLayoutDesc);
    if (mVertexShaderLayout == VK_NULL_HANDLE)
        return false;

    std::vector<DescriptorSetLayoutDesc> fsLayoutDesc;
    fsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, mFragmentShaderSampler});
    mFragmentShaderLayout = CreateLayout(fsLayoutDesc);
    if (mFragmentShaderLayout == VK_NULL_HANDLE)
        return false;

    return true;
}

void DescriptorLayoutManager::Release()
{
    if (mFragmentShaderLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(mDevice, mFragmentShaderLayout, nullptr);
        mFragmentShaderLayout = VK_NULL_HANDLE;
    }

    if (mVertexShaderLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(mDevice, mVertexShaderLayout, nullptr);
        mVertexShaderLayout = VK_NULL_HANDLE;
    }

    if (mFragmentShaderSampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(mDevice, mFragmentShaderSampler, nullptr);
        mFragmentShaderSampler = VK_NULL_HANDLE;
    }
}

} // namespace Renderer
} // namespace ABench
