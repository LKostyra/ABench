#pragma once

#include "Prerequisites.hpp"
#include "Device.hpp"
#include "Buffer.hpp"


namespace ABench {
namespace Renderer {

struct DescriptorSetLayoutDesc
{
    VkDescriptorType type;
    VkShaderStageFlags stage;
    VkSampler sampler;

    DescriptorSetLayoutDesc()
        : type(VK_DESCRIPTOR_TYPE_SAMPLER)
        , stage(0)
        , sampler(VK_NULL_HANDLE)
    {
    }

    DescriptorSetLayoutDesc(VkDescriptorType type, VkShaderStageFlags stage, VkSampler sampler)
        : type(type)
        , stage(stage)
        , sampler(sampler)
    {
    }
};

/**
 * A collection of "tools" - single functions creating Vulkan object in one-line.
 *
 * This serves as a wrapper for Vk*CreateInfo structures, to make all repeatable code more
 * clear and easy to use. It is caller's duty to free created resources.
 *
 * Since most tools return object references, returned VK_NULL_HANDLE is treated as error.
 */
class Tools
{
public:
    Tools();
    ~Tools();

    // VkRenderPass creation
    VkRenderPass CreateRenderPass(VkFormat colorFormat, VkFormat depthFormat);

    // VkPipelineLayout creation, sets and setCount can be null/zero.
    VkPipelineLayout CreatePipelineLayout(VkDescriptorSetLayout* sets = nullptr, int setCount = 0);

    // VkDescriptorPool creation.
    VkDescriptorPool CreateDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes);

    // VkDescriptorSetLayout creation, VkSampler is optional
    VkDescriptorSetLayout CreateDescriptorSetLayout(std::vector<DescriptorSetLayoutDesc>& descriptors);

    // Descriptor Sets allocation
    VkDescriptorSet AllocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout layout);

    // Updating descriptor sets
    void UpdateBufferDescriptorSet(VkDescriptorSet set, VkDescriptorType type, uint32_t binding, VkBuffer buffer, VkDeviceSize size);
    void UpdateTextureDescriptorSet(VkDescriptorSet set, VkDescriptorType type, uint32_t binding, VkImageView view);

    // Fence creation
    VkFence CreateFence();

    // Sampler creation
    VkSampler CreateSampler();
};

} // namespace Renderer
} // namespace ABench