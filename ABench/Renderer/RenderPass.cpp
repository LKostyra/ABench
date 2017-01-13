#include "../PCH.hpp"
#include "RenderPass.hpp"
#include "Util.hpp"
#include "Extensions.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

RenderPass::RenderPass(const Device* device)
    : mDevicePtr(device)
    , mRenderPass(VK_NULL_HANDLE)
{
}

RenderPass::~RenderPass()
{
    if (mRenderPass)
        vkDestroyRenderPass(mDevicePtr->mDevice, mRenderPass, nullptr);
}

bool RenderPass::Init(const RenderPassDesc& desc)
{
    // TODO multiple color attachments
    VkAttachmentDescription atts[2];
    uint32_t attCount = 0;
    ZERO_MEMORY(atts[attCount]);
    atts[attCount].format = desc.colorFormat;
    atts[attCount].samples = VK_SAMPLE_COUNT_1_BIT;
    atts[attCount].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
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
    if (desc.depthFormat != VK_FORMAT_UNDEFINED)
    {
        ZERO_MEMORY(atts[attCount]);
        atts[attCount].format = desc.depthFormat;
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
    if (desc.depthFormat != VK_FORMAT_UNDEFINED)
        subpass.pDepthStencilAttachment = &depthRef;

    VkRenderPassCreateInfo rpInfo;
    ZERO_MEMORY(rpInfo);
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = attCount;
    rpInfo.pAttachments = atts;
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &subpass;

    VkResult result = vkCreateRenderPass(mDevicePtr->mDevice, &rpInfo, nullptr, &mRenderPass);
    CHECK_VKRESULT(result, "Failed to create Render Pass");

    return true;
}

} // namespace Renderer
} // namespace ABench
