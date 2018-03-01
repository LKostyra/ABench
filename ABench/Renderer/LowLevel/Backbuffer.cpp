#include "PCH.hpp"
#include "Backbuffer.hpp"

#include "Util.hpp"
#include "Extensions.hpp"
#include "Translations.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

Backbuffer::Backbuffer()
    : mInstance()
    , mDevice()
    , mSurface(VK_NULL_HANDLE)
    , mPresentQueueIndex(0)
    , mPresentQueue(VK_NULL_HANDLE)
    , mColorSpace(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    , mPresentMode(VK_PRESENT_MODE_FIFO_KHR)
    , mSwapchain(VK_NULL_HANDLE)
    , mBufferCount(0)
{
}

Backbuffer::~Backbuffer()
{
    if (mSwapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(mDevice->GetDevice(), mSwapchain, nullptr);
    if (mSurface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(mInstance->GetVkInstance(), mSurface, nullptr);

    mInstance = nullptr;
}

bool Backbuffer::GetPresentQueue()
{
     // TODO to be replaced by Queue Manager
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(mDevice->mPhysicalDevice, &queueCount, nullptr);
    if (queueCount == 0)
    {
        LOGE("Physical device does not have any queue family properties.");
        return false;
    }
    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(mDevice->mPhysicalDevice, &queueCount, queueProps.data());

    for (mPresentQueueIndex = 0; mPresentQueueIndex < queueCount; mPresentQueueIndex++)
    {
        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(mDevice->mPhysicalDevice, mPresentQueueIndex,
                                             mSurface, &presentSupport);
        if (presentSupport)
            break;
    }

    if (mPresentQueueIndex == queueCount)
    {
        LOGE("No queue with present support!");
        return false;
    }

    LOGD("Selected queue #" << mPresentQueueIndex << " with Present support");
    vkGetDeviceQueue(mDevice->GetDevice(), mPresentQueueIndex, 0, &mPresentQueue);

    return true;
}

bool Backbuffer::SelectSurfaceFormat(const BackbufferDesc& desc)
{
    // Surface format selection
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->mPhysicalDevice, mSurface, &formatCount, nullptr);
    if (formatCount == 0)
    {
        LOGE("No surface formats to choose from.");
        return false;
    }

    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->mPhysicalDevice, mSurface,
                                                           &formatCount, surfaceFormats.data());
    RETURN_FALSE_IF_FAILED(result, "Failed to acquire physical device surface formats");

    uint32_t formatIndex = 0;
    if ((formatCount > 1) || (surfaceFormats[0].format != VK_FORMAT_UNDEFINED))
        for (formatIndex = 0; formatIndex < formatCount; ++formatIndex)
            if (surfaceFormats[formatIndex].format == desc.requestedFormat)
                break;

    if (formatIndex == formatCount)
    {
        LOGE("Requested surface format " << desc.requestedFormat << " ("
          << TranslateVkFormatToString(desc.requestedFormat) << ") is not supported by device.");
        LOGI("Available surface formats:");
        for (auto& format: surfaceFormats)
            LOGI("  " << format.format << " (" << TranslateVkFormatToString(format.format) << ")");
        return false;
    }

    mFormat = surfaceFormats[formatIndex].format;
    mColorSpace = surfaceFormats[formatIndex].colorSpace;

    return true;
}

bool Backbuffer::SelectPresentMode(const BackbufferDesc& desc)
{
    // Present mode selection
    uint32_t presentModeCount = UINT32_MAX;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->mPhysicalDevice, mSurface,
                                                                &presentModeCount, nullptr);
    RETURN_FALSE_IF_FAILED(result, "Failed to acquire surface's present mode count");
    if ((presentModeCount == 0) || (presentModeCount == UINT32_MAX))
    {
        LOGE("Failed to get present mode count for currently acquired surface");
        return false;
    }

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->mPhysicalDevice, mSurface,
                                                       &presentModeCount, presentModes.data());
    RETURN_FALSE_IF_FAILED(result, "Failed to acquire surface's present modes");

    // selection time
    // the default present mode will be FIFO (vsync on)
    mPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!desc.vsync)
    {
        // with vsync off, go through all the options and choose the best one
        for (uint32_t i = 0; i < presentModeCount; ++i)
        {
            // mailbox does not produce tearing, so take it if possible
            if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                mPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }

            // immediate produces tearing and should be chosen when mailbox is not available
            if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
                mPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }

    // at the end, inform if we actually achieved vsync
    if (!desc.vsync)
    {
        if (mPresentMode == VK_PRESENT_MODE_FIFO_KHR)
            LOGW("Unable to select non-VSync present mode. V-Sync will stay on.");
        else if (mPresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            LOGW("Mailbox present mode was unavailable. Selecting immediate mode, you may experience tearing.");
    }

    return true;
}

bool Backbuffer::AcquireSurfaceCaps()
{
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->mPhysicalDevice, mSurface, &mSurfCaps);
    RETURN_FALSE_IF_FAILED(result, "Failed to extract surface's capabilities");
    return true;
}

void Backbuffer::SelectBufferCount(const BackbufferDesc& desc)
{
    mBufferCount = desc.bufferCount;
    if ((mSurfCaps.maxImageCount > 0) && (mBufferCount > mSurfCaps.maxImageCount))
    {
        LOGW("Requested buffer count " << desc.bufferCount << " is too much - reducing to " << mSurfCaps.maxImageCount);
        mBufferCount = mSurfCaps.maxImageCount;
    }
    if (mBufferCount < mSurfCaps.minImageCount)
    {
        LOGW("Requested buffer count " << desc.bufferCount << " is too little - increasing to " << mSurfCaps.minImageCount);
        mBufferCount = mSurfCaps.minImageCount;
    }
}

bool Backbuffer::CreateSwapchain(const BackbufferDesc& desc)
{
    // create swapchain
    VkSwapchainCreateInfoKHR chainInfo;
    ZERO_MEMORY(chainInfo);
    chainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    chainInfo.surface = mSurface;
    chainInfo.minImageCount = mBufferCount;
    chainInfo.imageFormat = mFormat;
    chainInfo.imageColorSpace = mColorSpace;
    chainInfo.imageExtent.width = desc.width;
    chainInfo.imageExtent.height = desc.height;
    chainInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    chainInfo.preTransform = mSurfCaps.currentTransform;
    chainInfo.imageArrayLayers = 1;
    chainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    chainInfo.presentMode = mPresentMode;
    chainInfo.oldSwapchain = VK_NULL_HANDLE;
    chainInfo.clipped = VK_TRUE;
    chainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    VkResult result = vkCreateSwapchainKHR(mDevice->GetDevice(), &chainInfo, nullptr, &mSwapchain);
    RETURN_FALSE_IF_FAILED(result, "Failed to create swapchain");
    return true;
}

bool Backbuffer::AllocateImageViews()
{
    uint32_t currentBufferCount = 0;
    VkResult result = vkGetSwapchainImagesKHR(mDevice->GetDevice(), mSwapchain,
                                              &currentBufferCount, nullptr);
    RETURN_FALSE_IF_FAILED(result, "Failed to acquire Swapchain image count");

    if (currentBufferCount != mBufferCount)
    {
        LOGW("Allocated " << currentBufferCount << " images instead of " << mBufferCount);
        mBufferCount = currentBufferCount;
    }

    mImages.resize(mBufferCount);

    std::vector<VkImage> images(mBufferCount);
    result = vkGetSwapchainImagesKHR(mDevice->GetDevice(), mSwapchain,
                                              &mBufferCount, images.data());
    RETURN_FALSE_IF_FAILED(result, "Failed to acquire Swapchain images");

    for (uint32_t i = 0; i < mBufferCount; ++i)
        mImages[i].image = images[i];

    LOGD(mBufferCount << " swapchain images acquired.");

    ZERO_MEMORY(mSubresourceRange);
    mSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    mSubresourceRange.baseMipLevel = 0;
    mSubresourceRange.levelCount = 1;
    mSubresourceRange.baseArrayLayer = 0;
    mSubresourceRange.layerCount = 1;

    mDefaultLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    return true;
}

void Backbuffer::Transition(CommandBuffer* cmdBuffer, VkPipelineStageFlags fromStage, VkPipelineStageFlags toStage,
                            VkAccessFlags fromAccess, VkAccessFlags toAccess,
                            uint32_t fromQueueFamily, uint32_t toQueueFamily,
                            VkImageLayout targetLayout)
{
    if (targetLayout == mImages[mCurrentBuffer].layout)
        return; // no need to transition if we already have requested layout

    VkImageMemoryBarrier barrier;
    ZERO_MEMORY(barrier);
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = mImages[mCurrentBuffer].image;
    barrier.subresourceRange = mSubresourceRange;
    barrier.oldLayout = mImages[mCurrentBuffer].layout;
    barrier.newLayout = targetLayout;
    barrier.srcAccessMask = fromAccess;
    barrier.dstAccessMask = toAccess;
    barrier.srcQueueFamilyIndex = fromQueueFamily;
    barrier.dstQueueFamilyIndex = toQueueFamily;

    vkCmdPipelineBarrier(cmdBuffer->mCommandBuffer, fromStage, toStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    mImages[mCurrentBuffer].layout = targetLayout;
}

bool Backbuffer::Init(const DevicePtr& device, const BackbufferDesc& desc)
{
    mInstance = desc.instancePtr;
    mDevice = device;

    mWidth = desc.width;
    mHeight = desc.height;

    if (!CreateSurface(desc)) return false;
    if (!GetPresentQueue()) return false;
    if (!SelectSurfaceFormat(desc)) return false;
    if (!SelectPresentMode(desc)) return false;
    if (!AcquireSurfaceCaps()) return false;
    SelectBufferCount(desc);
    if (!CreateSwapchain(desc)) return false;
    if (!AllocateImageViews()) return false;

    if (!mCopyCommandBuffer.Init(mDevice, DeviceQueueType::TRANSFER))
        return false;

    mCopySemaphore = Tools::CreateSem(mDevice);
    if (!mCopySemaphore)
        return false;

    mCopyFence = Tools::CreateFence(mDevice, true);
    if (!mCopyFence)
        return false;

    LOGI("Backbuffer initialized successfully");
    return true;
}

bool Backbuffer::AcquireNextImage(VkSemaphore semaphore)
{
    VkResult result = vkAcquireNextImageKHR(mDevice->GetDevice(), mSwapchain, UINT64_MAX,
                                            semaphore, VK_NULL_HANDLE, &mCurrentBuffer);
    RETURN_FALSE_IF_FAILED(result, "Failed to preacquire next image for presenting");

    return true;
}

bool Backbuffer::Present(Texture& texture, VkSemaphore waitSemaphore)
{
    // wait until our previous copy finishes
    VkFence fences[] = { mCopyFence };
    VkResult result = vkWaitForFences(mDevice->GetDevice(), 1, fences, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS)
        LOGW("Failed to wait for Backbuffer fence: " << result << " (" << TranslateVkResultToString(result) << ")");

    result = vkResetFences(mDevice->GetDevice(), 1, fences);
    if (result != VK_SUCCESS)
        LOGW("Failed to reset Backbuffer fence: " << result << " (" << TranslateVkResultToString(result) << ")");

    // copy provided texture to backbuffer
    mCopyCommandBuffer.Begin();

    texture.Transition(&mCopyCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       0, VK_ACCESS_TRANSFER_READ_BIT,
                       VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    Transition(&mCopyCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       0, VK_ACCESS_TRANSFER_WRITE_BIT,
                       VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    mCopyCommandBuffer.CopyTextureToBackbuffer(&texture, this);

    texture.Transition(&mCopyCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       0, VK_ACCESS_TRANSFER_READ_BIT,
                       VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    Transition(&mCopyCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       0, VK_ACCESS_TRANSFER_WRITE_BIT,
                       VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                       VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    if (!mCopyCommandBuffer.End())
        return false;

    VkPipelineStageFlags waitFlags[] = { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
    VkSemaphore waitSemaphores[] = { waitSemaphore };
    mDevice->Execute(DeviceQueueType::TRANSFER, &mCopyCommandBuffer, 1,
                     waitFlags, waitSemaphores, mCopySemaphore, mCopyFence);


    VkSemaphore waitSem[] = { mCopySemaphore };
    // FIXME Closing window with Alt+F4/Close button causes swapchain to go
    //       out of date. Keep that in mind while supporting window resizing.
    VkPresentInfoKHR presentInfo;
    ZERO_MEMORY(presentInfo);
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mSwapchain;
    presentInfo.pImageIndices = &mCurrentBuffer;
    presentInfo.pWaitSemaphores = waitSem;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pResults = &result;
    vkQueuePresentKHR(mPresentQueue, &presentInfo);
    RETURN_FALSE_IF_FAILED(result, "Failed to present rendered image on screen");

    return true;
}

} // namespace Renderer
} // namespace ABench
