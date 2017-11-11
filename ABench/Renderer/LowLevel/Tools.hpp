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
    // hide ctor/dtor - this class should only provide static methods
    Tools() {};
    ~Tools() {};

public:
    // Fence creation
    static VkFence CreateFence(const DevicePtr& device);

    // Descriptor Set Layout creation
    static VkDescriptorSetLayout CreateDescriptorSetLayout(const DevicePtr& device, const std::vector<DescriptorSetLayoutDesc>& descriptors);

    // VkPipelineLayout creation, sets and setCount can be null/zero.
    static VkPipelineLayout CreatePipelineLayout(const DevicePtr& device, const std::vector<VkDescriptorSetLayout>& setLayouts);

    // VkRenderPass creation
    static VkRenderPass CreateRenderPass(const DevicePtr& device, VkFormat colorFormat, VkFormat depthFormat);

    // Updating descriptor sets
    static void UpdateBufferDescriptorSet(const DevicePtr& device, VkDescriptorSet set, VkDescriptorType type, uint32_t binding, VkBuffer buffer, VkDeviceSize size);
    static void UpdateTextureDescriptorSet(const DevicePtr& device, VkDescriptorSet set, VkDescriptorType type, uint32_t binding, VkImageView view);
};

} // namespace Renderer
} // namespace ABench