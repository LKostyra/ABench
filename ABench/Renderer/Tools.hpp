#pragma once

#include "Device.hpp"
#include "Buffer.hpp"


namespace ABench {
namespace Renderer {

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
    Device* mDevicePtr;

public:
    Tools();
    ~Tools();

    bool Init(Device* devicePtr);

    // VkRenderPass creation
    VkRenderPass CreateRenderPass(VkFormat colorFormat, VkFormat depthFormat);

    // VkPipelineLayout creation, sets and setCount can be null/zero.
    VkPipelineLayout CreatePipelineLayout(VkDescriptorSetLayout* sets = nullptr, int setCount = 0);

    // VkDescriptorPool creation.
    VkDescriptorPool CreateDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes);

    // VkDescriptorSetLayout creation, VkSampler is optional
    VkDescriptorSetLayout CreateDescriptorSetLayout(VkDescriptorType type, VkShaderStageFlags stage, VkSampler sampler = VK_NULL_HANDLE);

    // Descriptor Sets allocation
    VkDescriptorSet AllocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout layout);

    // Updating descriptor sets
    void UpdateBufferDescriptorSet(VkDescriptorSet set, VkDescriptorType type, const Buffer& buffer);
};

} // namespace Renderer
} // namespace ABench