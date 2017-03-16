#pragma once

#include "Device.hpp"
#include <list>

namespace ABench {
namespace Renderer {

// TODO the concept of garbage collection here might be problematic in the future.
//      Consider if it's actually needed, or if manual resource control should be done.
/**
 * A collection of "tools" - single functions creating Vulkan object in one-line.
 *
 * This serves as a wrapper for Vk*CreateInfo structures, to make all repeatable code more
 * clear and easy to use. The class automatically keeps references of created objects, so
 * freeing will be done automagically.
 *
 * Since most tools return object references, returned VK_NULL_HANDLE is treated as error.
 */
class Tools
{
    const Device* mDeviceRef;

    std::list<VkPipelineLayout> mPipelineLayouts;
    std::list<VkDescriptorPool> mDescriptorPools;
    std::list<VkDescriptorSetLayout> mDescriptorLayouts;

public:
    Tools(const Device* device);
    ~Tools();

    // VkPipelineLayout creation. Sets and setCount can be null/zero.
    VkPipelineLayout CreatePipelineLayout(VkDescriptorSetLayout* sets, int setCount);

    // VkDescriptorPool creation.
    VkDescriptorPool CreateDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes);

    // VkDescriptorSetLayout creation. VkSampler is optional.
    VkDescriptorSetLayout CreateDescriptorSetLayout(VkDescriptorType type, VkShaderStageFlags stage, VkSampler sampler = VK_NULL_HANDLE);

    // Descriptor Sets allocation
    VkDescriptorSet AllocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout layout);
};

} // namespace Renderer
} // namespace ABench