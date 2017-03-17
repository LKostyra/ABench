#include "../PCH.hpp"
#include "Tools.hpp"

#include "Util.hpp"
#include "Extensions.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

Tools::Tools()
    : mDevicePtr(nullptr)
{
}

Tools::~Tools()
{
    mDevicePtr = nullptr;
}

bool Tools::Init(Device* devicePtr)
{
    if (devicePtr == nullptr)
    {
        LOGE("Valid pointer to Device must be provided");
        return false;
    }

    mDevicePtr = devicePtr;
    return true;
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
        atts[attCount].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
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

    VkResult result = vkCreateRenderPass(mDevicePtr->GetDevice(), &rpInfo, nullptr, &rp);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Render Pass");

    return rp;
}

VkPipelineLayout Tools::CreatePipelineLayout(VkDescriptorSetLayout* sets, int setCount)
{
    VkPipelineLayout layout;

    VkPipelineLayoutCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pSetLayouts = sets;
    info.setLayoutCount = setCount;
    VkResult result = vkCreatePipelineLayout(mDevicePtr->GetDevice(), &info, nullptr, &layout);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Pipeline Layout");

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
    VkResult result = vkCreateDescriptorPool(mDevicePtr->GetDevice(), &info, nullptr, &pool);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Descriptor Pool");

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
    VkResult result = vkCreateDescriptorSetLayout(mDevicePtr->GetDevice(), &info, nullptr, &layout);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Descriptor Set Layout");

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
    VkResult result = vkAllocateDescriptorSets(mDevicePtr->GetDevice(), &info, &set);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to allocate Descriptor Set");

    return set;
}

} // namespace Renderer
} // namespace ABench
