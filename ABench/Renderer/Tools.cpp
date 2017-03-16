#include "../PCH.hpp"
#include "Tools.hpp"

#include "Util.hpp"
#include "Extensions.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

Tools::Tools(const Device* device)
    : mDeviceRef(device)
{
}

Tools::~Tools()
{
    for (auto& pipeLayout: mPipelineLayouts)
        vkDestroyPipelineLayout(mDeviceRef->GetDevice(), pipeLayout, nullptr);
    for (auto& descLayout : mDescriptorLayouts)
        vkDestroyDescriptorSetLayout(mDeviceRef->GetDevice(), descLayout, nullptr);
    for (auto& descPool: mDescriptorPools)
        vkDestroyDescriptorPool(mDeviceRef->GetDevice(), descPool, nullptr);
}

VkPipelineLayout Tools::CreatePipelineLayout(VkDescriptorSetLayout* sets, int setCount)
{
    VkPipelineLayout layout;

    VkPipelineLayoutCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pSetLayouts = sets;
    info.setLayoutCount = setCount;
    VkResult result = vkCreatePipelineLayout(mDeviceRef->GetDevice(), &info, nullptr, &layout);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Pipeline Layout");

    mPipelineLayouts.push_back(layout);
    return layout;
}

VkDescriptorPool Tools::CreateDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes)
{
    VkDescriptorPool pool = VK_NULL_HANDLE;

    VkDescriptorPoolCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.pPoolSizes = poolSizes.data();
    info.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    VkResult result = vkCreateDescriptorPool(mDeviceRef->GetDevice(), &info, nullptr, &pool);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Descriptor Pool");

    mDescriptorPools.push_back(pool);
    return pool;
}

VkDescriptorSetLayout Tools::CreateDescriptorSetLayout(VkDescriptorType type, VkShaderStageFlags stage, VkSampler sampler)
{
    VkDescriptorSetLayout layout;

    VkDescriptorSetLayoutBinding binding;
    ZERO_MEMORY(binding);
    binding.descriptorCount = 1;
    binding.binding = 0;
    binding.descriptorType = type;
    binding.stageFlags = stage;
    if (sampler != VK_NULL_HANDLE)
        binding.pImmutableSamplers = &sampler;

    VkDescriptorSetLayoutCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 1;
    info.pBindings = &binding;
    VkResult result = vkCreateDescriptorSetLayout(mDeviceRef->GetDevice(), &info, nullptr, &layout);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Descriptor Set Layout");
    
    mDescriptorLayouts.push_back(layout);
    return layout;
}

VkDescriptorSet Tools::AllocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout layout)
{
    VkDescriptorSet set;

    VkDescriptorSetAllocateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorPool = pool;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &layout;
    VkResult result = vkAllocateDescriptorSets(mDeviceRef->GetDevice(), &info, &set);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to allocate Descriptor Set");

    // no resource gathering here, as descriptor sets are freed with pool
    return set;
}

} // namespace Renderer
} // namespace ABench
