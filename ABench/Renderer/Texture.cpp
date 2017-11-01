#include "PCH.hpp"
#include "Texture.hpp"

#include "Util.hpp"
#include "Extensions.hpp"
#include "CommandBuffer.hpp"
#include "Renderer.hpp"
#include "Tools.hpp"
#include "DescriptorLayoutManager.hpp"

#include "Common/Common.hpp"


namespace ABench {
namespace Renderer {

Texture::Texture()
    : mWidth(0)
    , mHeight(0)
    , mFormat(VK_FORMAT_UNDEFINED)
    , mFromSwapchain(false)
    , mCurrentBuffer(0)
    , mDefaultLayout(VK_IMAGE_LAYOUT_UNDEFINED)
    , mImageDescriptorSet(VK_NULL_HANDLE)
{
}

Texture::~Texture()
{
    for (auto& image: mImages)
    {
        if (image.memory != VK_NULL_HANDLE)
            vkFreeMemory(gDevice->GetDevice(), image.memory, nullptr);
        if (image.view != VK_NULL_HANDLE)
            vkDestroyImageView(gDevice->GetDevice(), image.view, nullptr);
        if (!mFromSwapchain && image.image != VK_NULL_HANDLE)
            vkDestroyImage(gDevice->GetDevice(), image.image, nullptr);
    }
}

bool Texture::Init(const TextureDesc& desc)
{
    mImages.resize(1);
    mCurrentBuffer = 0; // since we create a non-backbuffer texture, this should stay that way

    VkImageCreateInfo imageInfo;
    ZERO_MEMORY(imageInfo);
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.format = desc.format;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.mipLevels = desc.mipmapCount;
    imageInfo.extent.width = desc.width;
    imageInfo.extent.height = desc.height;
    imageInfo.extent.depth = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.usage = desc.usage;
    if (desc.data != nullptr) imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = mImages[mCurrentBuffer].currentLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    VkResult result = vkCreateImage(gDevice->GetDevice(), &imageInfo, nullptr, &mImages[0].image);
    RETURN_FALSE_IF_FAILED(result, "Failed to create Image for texture");

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(gDevice->GetDevice(), mImages[0].image, &memReqs);

    VkMemoryAllocateInfo memInfo;
    ZERO_MEMORY(memInfo);
    memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memInfo.memoryTypeIndex = gDevice->GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    memInfo.allocationSize = memReqs.size;
    result = vkAllocateMemory(gDevice->GetDevice(), &memInfo, nullptr, &mImages[0].memory);
    RETURN_FALSE_IF_FAILED(result, "Failed to allocate memory for Image");

    result = vkBindImageMemory(gDevice->GetDevice(), mImages[0].image, mImages[0].memory, 0);
    RETURN_FALSE_IF_FAILED(result, "Binding Image memory to Image failed");

    if (desc.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        mDefaultLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    else if (desc.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        mDefaultLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    else if (desc.usage & VK_IMAGE_USAGE_SAMPLED_BIT)
        mDefaultLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    ZERO_MEMORY(mSubresourceRange);
    if (desc.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        mSubresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    else
        mSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    mSubresourceRange.baseMipLevel = 0;
    mSubresourceRange.levelCount = desc.mipmapCount;
    mSubresourceRange.baseArrayLayer = 0;
    mSubresourceRange.layerCount = 1;

    mWidth = desc.width;
    mHeight = desc.height;
    mFormat = desc.format;

    Buffer tempBuffer;

    if (desc.data != nullptr)
    {
        uint32_t dataSize = 0;
        for (uint32_t i = 0; i < desc.mipmapCount; ++i)
            dataSize += desc.data[i].dataSize;

        if (dataSize > memReqs.size)
        {
            LOGE("Not enough memory to initialize texture");
            return false;
        }

        BufferDesc tempBufferDesc;
        tempBufferDesc.type = BufferType::Dynamic;
        tempBufferDesc.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        tempBufferDesc.dataSize = dataSize;
        if (!tempBuffer.Init(tempBufferDesc))
            return false;

        uint32_t offset = 0;
        for (uint32_t i = 0; i < desc.mipmapCount; ++i)
        {
            tempBuffer.Write(desc.data[i].data, desc.data[i].dataSize, offset);
            offset += desc.data[i].dataSize;
        }
    }

    // transition Image to desired layout and eventually copy data to buffer
    // TODO OPTIMIZE this uses graphics queue and waits; after implementing queue manager, switch to Transfer queue
    CommandBuffer transitionCmdBuffer;
    if (!transitionCmdBuffer.Init(gDevice->GetCommandPool(DeviceQueueType::GRAPHICS)))
        return false;

    transitionCmdBuffer.Begin();

    if (desc.data != nullptr)
    {
        Transition(transitionCmdBuffer.mCommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        transitionCmdBuffer.CopyBufferToTexture(&tempBuffer, this);
    }

    Transition(transitionCmdBuffer.mCommandBuffer);
    transitionCmdBuffer.End();

    // TODO this should happen on Transfer Queue (might involve copying which takes time)
    gDevice->Execute(DeviceQueueType::GRAPHICS, &transitionCmdBuffer);
    gDevice->Wait(DeviceQueueType::GRAPHICS); // TODO this should be removed


    VkImageViewCreateInfo ivInfo;
    ZERO_MEMORY(ivInfo);
    ivInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivInfo.image = mImages[0].image;
    ivInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivInfo.format = desc.format;
    ivInfo.subresourceRange = mSubresourceRange;
    ivInfo.components = {
        VK_COMPONENT_SWIZZLE_R,
        VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B,
        VK_COMPONENT_SWIZZLE_A,
    };
    result = vkCreateImageView(gDevice->GetDevice(), &ivInfo, nullptr, &mImages[0].view);
    RETURN_FALSE_IF_FAILED(result, "Failed to create Image View for Texture");

    if (desc.usage & VK_IMAGE_USAGE_SAMPLED_BIT)
    {
        mImageDescriptorSet = gDevice->GetDescriptorAllocator().AllocateDescriptorSet(
            DescriptorLayoutManager::Instance().GetFragmentShaderDiffuseTextureLayout()
        );
        if (mImageDescriptorSet == VK_NULL_HANDLE)
            return false;

        Tools::UpdateTextureDescriptorSet(mImageDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, mImages[0].view);
    }

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