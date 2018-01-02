#include "PCH.hpp"
#include "Tools.hpp"

#include "Util.hpp"
#include "Extensions.hpp"
#include "Renderer/HighLevel/Renderer.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

VkRAII<VkFence> Tools::CreateFence(const DevicePtr& device, bool signaled)
{
    VkFence fence;

    VkFenceCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (signaled)
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult result = vkCreateFence(device->GetDevice(), &info, nullptr, &fence);
    RETURN_EMPTY_VKRAII_IF_FAILED(VkFence, result, "Failed to create fence");

    return VkRAII<VkFence>(fence, [device](VkFence f) {
        vkDestroyFence(device->GetDevice(), f, nullptr);
    });
}

VkRAII<VkSemaphore> Tools::CreateSem(const DevicePtr& device)
{
    VkSemaphore sem;

    VkSemaphoreCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkResult result = vkCreateSemaphore(device->GetDevice(), &info, nullptr, &sem);
    RETURN_EMPTY_VKRAII_IF_FAILED(VkSemaphore, result, "Failed to create semaphore");

    return VkRAII<VkSemaphore>(sem, [device](VkSemaphore s) {
        vkDestroySemaphore(device->GetDevice(), s, nullptr);
    });
}

VkRAII<VkDescriptorSetLayout> Tools::CreateDescriptorSetLayout(const DevicePtr& device, const std::vector<DescriptorSetLayoutDesc>& descriptors)
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
    VkResult result = vkCreateDescriptorSetLayout(device->GetDevice(), &info, nullptr, &layout);
    RETURN_EMPTY_VKRAII_IF_FAILED(VkDescriptorSetLayout, result, "Failed to create Descriptor Set Layout");

    return VkRAII<VkDescriptorSetLayout>(layout, [device](VkDescriptorSetLayout dsl) {
        vkDestroyDescriptorSetLayout(device->GetDevice(), dsl, nullptr);
    });
}

VkRAII<VkPipelineLayout> Tools::CreatePipelineLayout(const DevicePtr& device, const std::vector<VkDescriptorSetLayout>& setLayouts)
{
    VkPipelineLayout layout = VK_NULL_HANDLE;

    VkPipelineLayoutCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pSetLayouts = setLayouts.data();
    info.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    VkResult result = vkCreatePipelineLayout(device->GetDevice(), &info, nullptr, &layout);
    RETURN_EMPTY_VKRAII_IF_FAILED(VkPipelineLayout, result, "Failed to create Pipeline Layout");

    return VkRAII<VkPipelineLayout>(layout, [device](VkPipelineLayout pl) {
        vkDestroyPipelineLayout(device->GetDevice(), pl, nullptr);
    });
}

VkAttachmentDescription Tools::CreateAttachmentDescription(VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
                                                           VkImageLayout initialLayout, VkImageLayout finalLayout)
{
    VkAttachmentDescription att;

    ZERO_MEMORY(att);
    att.format = format;
    att.samples = VK_SAMPLE_COUNT_1_BIT;
    att.loadOp = loadOp;
    att.storeOp = storeOp;
    att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att.initialLayout = initialLayout;
    att.finalLayout = finalLayout;

    return att;
}

VkAttachmentReference Tools::CreateAttachmentReference(uint32_t attachment, VkImageLayout layout)
{
    VkAttachmentReference ref;

    ZERO_MEMORY(ref);
    ref.attachment = attachment;
    ref.layout = layout;

    return ref;
}

VkSubpassDescription Tools::CreateSubpass(const std::vector<VkAttachmentReference>& colorAttRefs, VkAttachmentReference* depthAttRef)
{
    VkSubpassDescription subpass;

    ZERO_MEMORY(subpass);
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttRefs.size());
    subpass.pColorAttachments = colorAttRefs.data();
    subpass.pDepthStencilAttachment = depthAttRef;

    return subpass;
}

VkSubpassDependency Tools::CreateSubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass, VkPipelineStageFlags srcStage,
                                                   VkPipelineStageFlags dstStage, VkAccessFlags srcAccess, VkAccessFlags dstAccess)
{
    VkSubpassDependency dep;

    ZERO_MEMORY(dep);
    dep.srcSubpass = srcSubpass;
    dep.dstSubpass = dstSubpass;
    dep.srcStageMask = srcStage;
    dep.dstStageMask = dstStage;
    dep.srcAccessMask = srcAccess;
    dep.dstAccessMask = dstAccess;
    dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    return dep;
}

VkRAII<VkRenderPass> Tools::CreateRenderPass(const DevicePtr& device, const std::vector<VkAttachmentDescription>& attachments,
                                             const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& subpassDeps)
{
    VkRenderPass rp = VK_NULL_HANDLE;

    VkRenderPassCreateInfo rpInfo;
    ZERO_MEMORY(rpInfo);
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    rpInfo.pAttachments = attachments.data();
    rpInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
    rpInfo.pSubpasses = subpasses.data();
    rpInfo.dependencyCount = static_cast<uint32_t>(subpassDeps.size());
    rpInfo.pDependencies = subpassDeps.data();

    VkResult result = vkCreateRenderPass(device->GetDevice(), &rpInfo, nullptr, &rp);
    RETURN_EMPTY_VKRAII_IF_FAILED(VkRenderPass, result, "Failed to create Render Pass");

    return VkRAII<VkRenderPass>(rp, [device](VkRenderPass rp) {
        vkDestroyRenderPass(device->GetDevice(), rp, nullptr);
    });
}

VkRAII<VkSampler> Tools::CreateSampler(const DevicePtr& device)
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
    VkResult result = vkCreateSampler(device->GetDevice(), &sampInfo, nullptr, &sampler);
    RETURN_EMPTY_VKRAII_IF_FAILED(VkSampler, result, "Failed to create Sampler");

    return VkRAII<VkSampler>(sampler, [device](VkSampler s) {
        vkDestroySampler(device->GetDevice(), s, nullptr);
    });
}

void Tools::UpdateBufferDescriptorSet(const DevicePtr& device, VkDescriptorSet set, VkDescriptorType type, uint32_t binding, VkBuffer buffer, VkDeviceSize size)
{
    VkDescriptorBufferInfo info;
    ZERO_MEMORY(info);
    info.buffer = buffer;
    info.range = size;

    VkWriteDescriptorSet write;
    ZERO_MEMORY(write);
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorType = type;
    write.descriptorCount = 1;
    write.pBufferInfo = &info;

    vkUpdateDescriptorSets(device->GetDevice(), 1, &write, 0, nullptr);
}

void Tools::UpdateTextureDescriptorSet(const DevicePtr& device, VkDescriptorSet set, VkDescriptorType type, uint32_t binding, VkImageView view)
{
    VkDescriptorImageInfo imgInfo;
    ZERO_MEMORY(imgInfo);
    imgInfo.imageView = view;

    VkWriteDescriptorSet writeSet;
    ZERO_MEMORY(writeSet);
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstSet = set;
    writeSet.dstBinding = binding;
    writeSet.dstArrayElement = 0;
    writeSet.descriptorCount = 1;
    writeSet.descriptorType = type;
    writeSet.pImageInfo = &imgInfo;

    vkUpdateDescriptorSets(device->GetDevice(), 1, &writeSet, 0, nullptr);
}

} // namespace Renderer
} // namespace ABench
