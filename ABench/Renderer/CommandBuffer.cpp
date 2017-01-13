#include "../PCH.hpp"
#include "CommandBuffer.hpp"
#include "Util.hpp"
#include "Extensions.hpp"

#include "Common/Common.hpp"


namespace ABench {
namespace Renderer {

CommandBuffer::CommandBuffer(const Device* device)
    : mDevicePtr(device)
{
}

CommandBuffer::~CommandBuffer()
{
    if (mCommandBuffer)
        vkFreeCommandBuffers(mDevicePtr->mDevice, mDevicePtr->mCommandPool, 1, &mCommandBuffer);
}

bool CommandBuffer::Init()
{
    VkCommandBufferAllocateInfo allocInfo;
    ZERO_MEMORY(allocInfo);
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = mDevicePtr->mCommandPool;
    VkResult result = vkAllocateCommandBuffers(mDevicePtr->mDevice, &allocInfo, &mCommandBuffer);
    CHECK_VKRESULT(result, "Failed to allocate Command Buffer");

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

void CommandBuffer::BeginRenderPass(RenderPass* rp, Framebuffer* fb)
{
    VkRenderPassBeginInfo rpInfo;
    ZERO_MEMORY(rpInfo);
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass = rp->mRenderPass;
    rpInfo.renderArea.extent.width = fb->mWidth;
    rpInfo.renderArea.extent.height = fb->mHeight;
    rpInfo.framebuffer = fb->mFramebuffers[*fb->mCurentBufferPtr];
    vkCmdBeginRenderPass(mCommandBuffer, )
}

void CommandBuffer::EndRenderPass()
{
}

bool CommandBuffer::End()
{
    VkResult result = vkEndCommandBuffer(mCommandBuffer);
    CHECK_VKRESULT(result, "Failure during Command Buffer recording");
    return true;
}

} // namespace Renderer
} // namespace ABench
