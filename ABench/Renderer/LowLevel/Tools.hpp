#pragma once

#include "Prerequisites.hpp"
#include "Device.hpp"
#include "Buffer.hpp"
#include "VkRAII.hpp"

#include "Common/Traits.hpp"


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
    Tools() = default;
    ~Tools() = default;

public:
    // Fence creation
    static VkRAII<VkFence> CreateFence(const DevicePtr& device, bool signaled);

    // Semaphore creation (we cannot call this "CreateSemaphore" >:( WinAPI has the same define )
    static VkRAII<VkSemaphore> CreateSem(const DevicePtr& device);

    // Descriptor Set Layout creation
    static VkRAII<VkDescriptorSetLayout> CreateDescriptorSetLayout(const DevicePtr& device, const std::vector<DescriptorSetLayoutDesc>& descriptors);

    // VkPipelineLayout creation, sets and setCount can be null/zero.
    static VkRAII<VkPipelineLayout> CreatePipelineLayout(const DevicePtr& device, const std::vector<VkDescriptorSetLayout>& setLayouts);

    // VkRenderPass creation
    static VkAttachmentDescription CreateAttachmentDescription(VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
                                                               VkImageLayout initialLayout, VkImageLayout finalLayout);
    static VkAttachmentReference CreateAttachmentReference(uint32_t attachment, VkImageLayout layout);
    static VkSubpassDescription CreateSubpass(const std::vector<VkAttachmentReference>& colorAttRefs, VkAttachmentReference* depthAttRef);
    static VkSubpassDependency CreateSubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass, VkPipelineStageFlags srcStage,
                                                       VkPipelineStageFlags dstStage, VkAccessFlags srcAccess, VkAccessFlags dstAccess);
    static VkRAII<VkRenderPass> CreateRenderPass(const DevicePtr& device, const std::vector<VkAttachmentDescription>& attachments,
                                                 const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& subpassDeps);
    // VkSampler creation
    static VkRAII<VkSampler> CreateSampler(const DevicePtr& device);

    // Updating descriptor sets
    static void UpdateBufferDescriptorSet(const DevicePtr& device, VkDescriptorSet set, VkDescriptorType type, uint32_t binding, VkBuffer buffer, VkDeviceSize size);
    static void UpdateTextureDescriptorSet(const DevicePtr& device, VkDescriptorSet set, VkDescriptorType type, uint32_t binding, VkImageView view);
};

} // namespace Renderer
} // namespace ABench