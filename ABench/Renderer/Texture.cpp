#include "../PCH.hpp"
#include "Texture.hpp"

#include "Util.hpp"
#include "Extensions.hpp"

#include "Common/Common.hpp"


namespace ABench {
namespace Renderer {

Texture::Texture()
    : mDevicePtr(nullptr)
{
}

Texture::~Texture()
{
    mDevicePtr = nullptr;
}

bool Texture::Init(const TextureDesc& desc)
{
    return true;
}

void Texture::Transition(VkCommandBuffer cmdBuffer, VkImageLayout targetLayout)
{
    if (targetLayout == VK_IMAGE_LAYOUT_UNDEFINED)
        targetLayout = mDefaultLayout;

    if (targetLayout == mImages[mCurrentBuffer].currentLayout)
        return; // no need to transition if we already have requested layout

    VkImageMemoryBarrier barrier;
    ZERO_MEMORY(barrier);
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = mImages[mCurrentBuffer].image;
    barrier.subresourceRange = mSubresourceRange;
    barrier.oldLayout = mImages[mCurrentBuffer].currentLayout;
    barrier.newLayout = targetLayout;
    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);

    mImages[mCurrentBuffer].currentLayout = targetLayout;
}

} // namespace Renderer
} // namespace ABench