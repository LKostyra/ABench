#include "PCH.hpp"
#include "Tools.hpp"

#include "Util.hpp"
#include "Extensions.hpp"
#include "Renderer/HighLevel/Renderer.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

VkFence Tools::CreateFence(const DevicePtr& device)
{
    VkFence fence;

    VkFenceCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkResult result = vkCreateFence(device->GetDevice(), &info, nullptr, &fence);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create fence");

    return fence;
}

VkDescriptorSetLayout Tools::CreateDescriptorSetLayout(const DevicePtr& device, const std::vector<DescriptorSetLayoutDesc>& descriptors)
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
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Descriptor Set Layout");

    LOGD("Created Descriptor Set Layout 0x" << std::hex << reinterpret_cast<size_t*>(layout) <<
         " with " << std::dec << bindings.size() << " bindings.");

    return layout;
}

VkPipelineLayout Tools::CreatePipelineLayout(const DevicePtr& device, const std::vector<VkDescriptorSetLayout>& setLayouts)
{
    VkPipelineLayout layout = VK_NULL_HANDLE;

    VkPipelineLayoutCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pSetLayouts = setLayouts.data();
    info.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    VkResult result = vkCreatePipelineLayout(device->GetDevice(), &info, nullptr, &layout);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Pipeline Layout");

    LOGD("Created Pipeline Layout 0x" << std::hex << reinterpret_cast<size_t*>(layout));
    return layout;
}

VkRenderPass Tools::CreateRenderPass(const DevicePtr& device, VkFormat colorFormat, VkFormat depthFormat)
{
    VkRenderPass rp = VK_NULL_HANDLE;

    // TODO multiple color attachments
    VkAttachmentDescription atts[2];
    uint32_t attCount = 0;
    ZERO_MEMORY(atts[attCount]);
    atts[attCount].format = colorFormat;
    atts[attCount].samples = VK_SAMPLE_COUNT_1_BIT;
    atts[attCount].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    atts[attCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    atts[attCount].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    atts[attCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[attCount].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    atts[attCount].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attCount++;

    VkAttachmentReference colorRef;
    ZERO_MEMORY(colorRef);
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef;
    if (depthFormat != VK_FORMAT_UNDEFINED)
    {
        ZERO_MEMORY(atts[attCount]);
        atts[attCount].format = depthFormat;
        atts[attCount].samples = VK_SAMPLE_COUNT_1_BIT;
        atts[attCount].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        atts[attCount].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        atts[attCount].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        atts[attCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        atts[attCount].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        atts[attCount].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attCount++;

        depthRef.attachment = 1;
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    // TODO multiple subpasses
    VkSubpassDescription subpass;
    ZERO_MEMORY(subpass);
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    if (depthFormat != VK_FORMAT_UNDEFINED)
        subpass.pDepthStencilAttachment = &depthRef;

    VkRenderPassCreateInfo rpInfo;
    ZERO_MEMORY(rpInfo);
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = attCount;
    rpInfo.pAttachments = atts;
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &subpass;

    VkResult result = vkCreateRenderPass(device->GetDevice(), &rpInfo, nullptr, &rp);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Render Pass");

    LOGD("Created Render Pass 0x" << std::hex << reinterpret_cast<size_t*>(rp));
    return rp;
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
