#include "../PCH.hpp"
#include "CommandBuffer.hpp"
#include "Util.hpp"
#include "Extensions.hpp"

#include "Common/Common.hpp"

#include <cstring>

namespace ABench {
namespace Renderer {

CommandBuffer::CommandBuffer(const Device* device)
    : mDevicePtr(device)
{
}

CommandBuffer::~CommandBuffer()
{
    if (mCommandBuffer)
        vkFreeCommandBuffers(mDevicePtr->GetDevice(), mDevicePtr->GetCommandPool(), 1, &mCommandBuffer);
}

bool CommandBuffer::Init()
{
    VkCommandBufferAllocateInfo allocInfo;
    ZERO_MEMORY(allocInfo);
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = mDevicePtr->GetCommandPool();
    VkResult result = vkAllocateCommandBuffers(mDevicePtr->GetDevice(), &allocInfo, &mCommandBuffer);
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

void CommandBuffer::BeginRenderPass(RenderPass* rp, Framebuffer* fb, float clearValues[4])
{
    VkClearValue clear;
    memcpy(clear.color.float32, clearValues, 4 * sizeof(float));

    VkRenderPassBeginInfo rpInfo;
    ZERO_MEMORY(rpInfo);
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass = rp->mRenderPass;
    rpInfo.renderArea.extent.width = fb->mWidth;
    rpInfo.renderArea.extent.height = fb->mHeight;
    rpInfo.clearValueCount = 1;
    rpInfo.pClearValues = &clear;
    rpInfo.framebuffer = fb->mFramebuffers[*(fb->mCurrentBufferPtr)];
    vkCmdBeginRenderPass(mCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

    mCurrentFramebuffer = fb;
}

void CommandBuffer::BindVertexBuffer(Buffer* buffer)
{
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(mCommandBuffer, 0, 1, &buffer->mBuffer, &offset);
}

void CommandBuffer::BindPipeline(Pipeline* pipeline)
{
    vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->mPipeline);
}

void CommandBuffer::Clear(float clearValues[4])
{
    VkClearAttachment clearAtt;

    clearAtt.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearAtt.colorAttachment = 0;
    memcpy(clearAtt.clearValue.color.float32, clearValues, 4 * sizeof(float));

    VkClearRect rect;
    rect.rect.offset = { 0, 0 };
    rect.rect.extent = { mCurrentFramebuffer->mWidth , mCurrentFramebuffer->mHeight };
    rect.baseArrayLayer = 0;
    rect.layerCount = 1;

    vkCmdClearAttachments(mCommandBuffer, 1, &clearAtt, 1, &rect);
}

void CommandBuffer::Draw(uint32_t vertCount)
{
    vkCmdDraw(mCommandBuffer, vertCount, 1, 0, 0);
}

void CommandBuffer::EndRenderPass()
{
    vkCmdEndRenderPass(mCommandBuffer);
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
