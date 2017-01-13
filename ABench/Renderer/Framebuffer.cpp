#include "../PCH.hpp"
#include "Framebuffer.hpp"
#include "Util.hpp"
#include "Extensions.hpp"

#include "Common/Common.hpp"
#include "Common/Logger.hpp"


namespace ABench {
namespace Renderer {

Framebuffer::Framebuffer(const Device* device)
    : mDevicePtr(device)
{
}

Framebuffer::~Framebuffer()
{
    if (mFramebuffers.size() > 0)
        for (auto& fb: mFramebuffers)
            vkDestroyFramebuffer(mDevicePtr->mDevice, fb, nullptr);
}

bool Framebuffer::Init(const FramebufferDesc& desc)
{
    mFramebuffers.resize(desc.colorTex->mImageViews.size());

    if (!desc.colorTex)
    {
        LOGE("Framebuffer requires at least a color texture");
        return false;
    }

    if ((desc.depthTex) && 
        ((desc.colorTex->mWidth != desc.depthTex->mWidth) ||
         (desc.colorTex->mHeight != desc.depthTex->mHeight)))
    {
        LOGE("Cannot create framebuffer - provided color and depth textures are incompatible");
        return false;
    }

    VkFramebufferCreateInfo fbInfo;
    ZERO_MEMORY(fbInfo);
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.width = desc.colorTex->mWidth;
    fbInfo.height = desc.colorTex->mHeight;
    fbInfo.renderPass = desc.renderPass->mRenderPass;
    fbInfo.layers = 1;

    VkResult result;
    VkImageView fbAtts[2];
    for (uint32_t i = 0; i < mFramebuffers.size(); ++i)
    {
        fbAtts[0] = desc.colorTex->mImageViews[i];
        fbInfo.attachmentCount = 1;
        if (desc.depthTex)
        {
            fbAtts[1] = desc.depthTex->mImageViews[0];
            fbInfo.attachmentCount = 2;
        }

        fbInfo.pAttachments = fbAtts;
        result = vkCreateFramebuffer(mDevicePtr->mDevice, &fbInfo, nullptr, &mFramebuffers[i]);
        CHECK_VKRESULT(result, "Failed to create Framebuffer #" << i << " for provided Texture");
    }

    mWidth = desc.colorTex->mWidth;
    mHeight = desc.colorTex->mHeight;
    return true;
}

} // namespace Renderer
} // namespace ABench
