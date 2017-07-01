#include "PCH.hpp"
#include "CommandBuffer.hpp"
#include "Util.hpp"
#include "Extensions.hpp"
#include "Renderer.hpp"

#include "Common/Common.hpp"

#include <cstring>

namespace ABench {
namespace Renderer {

CommandBuffer::CommandBuffer()
    : mCommandBuffer(VK_NULL_HANDLE)
    , mCurrentFramebuffer(nullptr)
{
}

CommandBuffer::~CommandBuffer()
{
    if (mCommandBuffer)
        vkFreeCommandBuffers(gDevice->GetDevice(), gDevice->GetCommandPool(), 1, &mCommandBuffer);
}

bool CommandBuffer::Init()
{
    VkCommandBufferAllocateInfo allocInfo;
    ZERO_MEMORY(allocInfo);
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = gDevice->GetCommandPool();
    VkResult result = vkAllocateCommandBuffers(gDevice->GetDevice(), &allocInfo, &mCommandBuffer);
    RETURN_FALSE_IF_FAILED(result, "Failed to allocate Command Buffer");

    return true;
}

void CommandBuffer::Begin()
{
    VkCommandBufferBeginInfo beginInfo;
    ZERO_MEMORY(beginInfo);
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(mCommandBuffer, &beginInfo);
}

void CommandBuffer::BeginRenderPass(VkRenderPass rp, Framebuffer* fb, ClearTypes types, float clearValues[4], float depthValue)
{
    VkClearValue clear[2];
    uint32_t clearCount = 0;

    if (types & ABENCH_CLEAR_COLOR)
    {
        memcpy(clear[clearCount].color.float32, clearValues, 4 * sizeof(float));
        clearCount++;
    }

    if (types & ABENCH_CLEAR_DEPTH)
    {
        clear[clearCount].depthStencil.depth = depthValue;
        clearCount++;
    }

    // Transition Framebuffer's texture to COLOR_ATTACHMENT layout
    fb->mTexturePtr->Transition(mCommandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    fb->mDepthTexturePtr->Transition(mCommandBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    VkRenderPassBeginInfo rpInfo;
    ZERO_MEMORY(rpInfo);
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass = rp;
    rpInfo.renderArea.extent.width = fb->mWidth;
    rpInfo.renderArea.extent.height = fb->mHeight;
    rpInfo.clearValueCount = clearCount;
    rpInfo.pClearValues = clear;
    rpInfo.framebuffer = fb->mFramebuffers[fb->mTexturePtr->GetCurrentBuffer()];
    vkCmdBeginRenderPass(mCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

    mCurrentFramebuffer = fb;
}

void CommandBuffer::BindVertexBuffer(const Buffer* buffer)
{
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(mCommandBuffer, 0, 1, &buffer->mBuffer, &offset);
}

void CommandBuffer::BindIndexBuffer(const Buffer* buffer)
{
    vkCmdBindIndexBuffer(mCommandBuffer, buffer->mBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void CommandBuffer::BindPipeline(Pipeline* pipeline)
{
    vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->mPipeline);
}

void CommandBuffer::BindDescriptorSet(VkDescriptorSet set, uint32_t setSlot, VkPipelineLayout layout)
{
    vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, setSlot, 1, &set, 0, nullptr);
}

void CommandBuffer::BindDescriptorSet(VkDescriptorSet set, uint32_t setSlot, VkPipelineLayout layout, uint32_t dynamicOffset)
{
    vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, setSlot, 1, &set, 1, &dynamicOffset);
}

void CommandBuffer::Clear(ClearTypes types, float clearValues[4], float depthValue)
{
    VkClearAttachment clearAtt[2];
    uint32_t clearAttCount = 0;

    if (types & ABENCH_CLEAR_COLOR)
    {
        ZERO_MEMORY(clearAtt[clearAttCount]);
        clearAtt[clearAttCount].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clearAtt[clearAttCount].colorAttachment = 0;
        memcpy(clearAtt[clearAttCount].clearValue.color.float32, clearValues, 4 * sizeof(float));
        clearAttCount++;
    }

    if (types & ABENCH_CLEAR_DEPTH)
    {
        ZERO_MEMORY(clearAtt[clearAttCount]);
        clearAtt[clearAttCount].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        clearAtt[clearAttCount].clearValue.depthStencil.depth = depthValue;
        clearAttCount++;
    }

    VkClearRect rect;
    rect.rect.offset = { 0, 0 };
    rect.rect.extent = { mCurrentFramebuffer->mWidth , mCurrentFramebuffer->mHeight };
    rect.baseArrayLayer = 0;
    rect.layerCount = 1;

    vkCmdClearAttachments(mCommandBuffer, clearAttCount, clearAtt, 1, &rect);
}

void CommandBuffer::CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
{
    VkBufferCopy region;
    ZERO_MEMORY(region);
    region.size = size;
    vkCmdCopyBuffer(mCommandBuffer, src, dst, 1, &region);
}

void CommandBuffer::CopyBufferToTexture(Buffer* src, Texture* dst)
{
    std::vector<VkBufferImageCopy> regions;

    uint32_t width = dst->mWidth;
    uint32_t height = dst->mHeight;
    uint32_t offset = 0;

    VkBufferImageCopy region;
    ZERO_MEMORY(region);
    for (uint32_t i = 0; i < dst->mSubresourceRange.levelCount; ++i)
    {
        region.bufferOffset = offset;
        region.imageExtent.width = width;
        region.imageExtent.height = height;
        region.imageExtent.depth = 1;
        region.imageSubresource.aspectMask = dst->mSubresourceRange.aspectMask;
        region.imageSubresource.baseArrayLayer = dst->mSubresourceRange.baseArrayLayer;
        region.imageSubresource.layerCount = dst->mSubresourceRange.layerCount;
        region.imageSubresource.mipLevel = i;

        regions.push_back(region);

        offset += width * height * TranslateVkFormatToFormatSize(dst->mFormat);
        width >>= 1;
        height >>= 1;
    }

    vkCmdCopyBufferToImage(mCommandBuffer, src->GetVkBuffer(), dst->GetVkImage(0), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(regions.size()), regions.data());
}

void CommandBuffer::Draw(uint32_t vertCount)
{
    vkCmdDraw(mCommandBuffer, vertCount, 1, 0, 0);
}

void CommandBuffer::DrawIndexed(uint32_t indexCount)
{
    vkCmdDrawIndexed(mCommandBuffer, indexCount, 1, 0, 0, 0);
}

void CommandBuffer::EndRenderPass()
{
    vkCmdEndRenderPass(mCommandBuffer);

    // revert texture to its default layout
    mCurrentFramebuffer->mTexturePtr->Transition(mCommandBuffer);
    mCurrentFramebuffer = nullptr;
}

bool CommandBuffer::End()
{
    VkResult result = vkEndCommandBuffer(mCommandBuffer);
    RETURN_FALSE_IF_FAILED(result, "Failure during Command Buffer recording");
    return true;
}

void CommandBuffer::SetViewport(int32_t x, int32_t y, uint32_t width, uint32_t height, float minDepth, float maxDepth)
{
    VkViewport viewport;
    viewport.x = static_cast<float>(x);
    viewport.y = static_cast<float>(y);
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
}

void CommandBuffer::SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    VkRect2D scissor;
    scissor.offset.x = x;
    scissor.offset.y = y;
    scissor.extent.width = width;
    scissor.extent.height = height;
    vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);
}

} // namespace Renderer
} // namespace ABench
