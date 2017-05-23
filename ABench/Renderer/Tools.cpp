#include "PCH.hpp"
#include "Tools.hpp"

#include "Util.hpp"
#include "Extensions.hpp"
#include "Renderer.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

Tools::Tools()
{
}

Tools::~Tools()
{
}

VkRenderPass Tools::CreateRenderPass(VkFormat colorFormat, VkFormat depthFormat)
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

    VkResult result = vkCreateRenderPass(gDevice->GetDevice(), &rpInfo, nullptr, &rp);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Render Pass");

    LOGD("Created Render Pass 0x" << std::hex << reinterpret_cast<size_t*>(rp));
    return rp;
}

VkPipelineLayout Tools::CreatePipelineLayout(VkDescriptorSetLayout* sets, int setCount)
{
    VkPipelineLayout layout = VK_NULL_HANDLE;

    VkPipelineLayoutCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pSetLayouts = sets;
    info.setLayoutCount = setCount;
    VkResult result = vkCreatePipelineLayout(gDevice->GetDevice(), &info, nullptr, &layout);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Pipeline Layout");

    LOGD("Created Pipeline Layout 0x" << std::hex << reinterpret_cast<size_t*>(layout));
    return layout;
}

VkDescriptorPool Tools::CreateDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes)
{
    VkDescriptorPool pool = VK_NULL_HANDLE;

    // gather maxSets from provided pool sizes
    uint32_t maxSets = 0;
    for (auto& size: poolSizes)
        maxSets += size.descriptorCount;

    VkDescriptorPoolCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.pPoolSizes = poolSizes.data();
    info.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    info.maxSets = maxSets;
    VkResult result = vkCreateDescriptorPool(gDevice->GetDevice(), &info, nullptr, &pool);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Descriptor Pool");

    LOGD("Created Descriptor Pool 0x" << std::hex << reinterpret_cast<size_t*>(pool));
    return pool;
}

VkDescriptorSetLayout Tools::CreateDescriptorSetLayout(std::vector<DescriptorSetLayoutDesc>& descriptors)
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
    VkResult result = vkCreateDescriptorSetLayout(gDevice->GetDevice(), &info, nullptr, &layout);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Descriptor Set Layout");

    LOGD("Created Descriptor Set Layout 0x" << std::hex << reinterpret_cast<size_t*>(layout) <<
         " with " << std::dec << bindings.size() << " bindings.");

    return layout;
}

VkDescriptorSet Tools::AllocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout layout)
{
    VkDescriptorSet set = VK_NULL_HANDLE;

    VkDescriptorSetAllocateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorPool = pool;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &layout;
    VkResult result = vkAllocateDescriptorSets(gDevice->GetDevice(), &info, &set);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to allocate Descriptor Set");

    LOGD("Allocated Descriptor Set 0x" << std::hex << reinterpret_cast<size_t*>(set) <<
         " from Pool 0x" << reinterpret_cast<size_t*>(pool));
    return set;
}

void Tools::UpdateBufferDescriptorSet(VkDescriptorSet set, VkDescriptorType type, uint32_t binding, VkBuffer buffer, VkDeviceSize size)
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

    vkUpdateDescriptorSets(gDevice->GetDevice(), 1, &write, 0, nullptr);
}

void Tools::UpdateTextureDescriptorSet(VkDescriptorSet set, VkDescriptorType type, uint32_t binding, VkImageView view)
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

    vkUpdateDescriptorSets(gDevice->GetDevice(), 1, &writeSet, 0, nullptr);
}

VkFence Tools::CreateFence()
{
    VkFence fence;

    VkFenceCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkResult result = vkCreateFence(gDevice->GetDevice(), &info, nullptr, &fence);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create fence");

    return fence;
}

VkSampler Tools::CreateSampler()
{
    VkSampler sampler;

    VkSamplerCreateInfo sampInfo;
    ZERO_MEMORY(sampInfo);
    sampInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampInfo.magFilter = VK_FILTER_LINEAR;
    sampInfo.minFilter = VK_FILTER_LINEAR;
    sampInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampInfo.mipLodBias = 0.0f;
    sampInfo.anisotropyEnable = VK_FALSE;
    sampInfo.maxAnisotropy = 0.0f;
    sampInfo.compareEnable = VK_FALSE;
    sampInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    sampInfo.minLod = FLT_MIN;
    sampInfo.maxLod = FLT_MAX;
    sampInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    sampInfo.unnormalizedCoordinates = VK_FALSE;
    VkResult result = vkCreateSampler(gDevice->GetDevice(), &sampInfo, nullptr, &sampler);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Sampler");

    return sampler;
}

} // namespace Renderer
} // namespace ABench
