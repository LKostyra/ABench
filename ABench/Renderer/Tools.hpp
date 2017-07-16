#pragma once

#include "Prerequisites.hpp"
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
    // hide ctor/dtor - this class should only provide static methods
    Tools() {};
    ~Tools() {};

public:
    // Fence creation
    static VkFence CreateFence();

    // VkPipelineLayout creation, sets and setCount can be null/zero.
    static VkPipelineLayout CreatePipelineLayout(VkDescriptorSetLayout* sets = nullptr, uint32_t setCount = 0);

    // VkRenderPass creation
    static VkRenderPass CreateRenderPass(VkFormat colorFormat, VkFormat depthFormat);

    // Updating descriptor sets
    static void UpdateBufferDescriptorSet(VkDescriptorSet set, VkDescriptorType type, uint32_t binding, VkBuffer buffer, VkDeviceSize size);
    static void UpdateTextureDescriptorSet(VkDescriptorSet set, VkDescriptorType type, uint32_t binding, VkImageView view);
};

} // namespace Renderer
} // namespace ABench