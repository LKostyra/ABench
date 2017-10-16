#include "PCH.hpp"
#include "Backbuffer.hpp"

#include "Util.hpp"
#include "Extensions.hpp"
#include "Translations.hpp"
#include "Renderer.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

Backbuffer::Backbuffer()
    : mInstancePtr(nullptr)
    , mSurface(VK_NULL_HANDLE)
    , mPresentQueueIndex(0)
    , mPresentQueue(VK_NULL_HANDLE)
    , mColorSpace(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    , mPresentMode(VK_PRESENT_MODE_FIFO_KHR)
    , mBufferCount(0)
    , mSwapchain(VK_NULL_HANDLE)
    , mImageAcquireFence(VK_NULL_HANDLE)
{
}

Backbuffer::~Backbuffer()
{
    if (mImageAcquireFence != VK_NULL_HANDLE)
        vkDestroyFence(gDevice->GetDevice(), mImageAcquireFence, nullptr);

    if (mSwapchain != VK_NULL_HANDLE)
    {
        for (auto& img: mImages)
        {
            if (img.view != VK_NULL_HANDLE)
            {
                vkDestroyImageView(gDevice->GetDevice(), img.view, nullptr);
                img.view = VK_NULL_HANDLE;
            }
        }
        vkDestroySwapchainKHR(gDevice->GetDevice(), mSwapchain, nullptr);
    }
    if (mSurface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(mInstancePtr->GetVkInstance(), mSurface, nullptr);

    mInstancePtr = nullptr;
}

bool Backbuffer::GetPresentQueue()
{
     // TODO to be replaced by Queue Manager
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gDevice->mPhysicalDevice, &queueCount, nullptr);
    if (queueCount == 0)
    {
        LOGE("Physical device does not have any queue family properties.");
        return false;
    }
    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(gDevice->mPhysicalDevice, &queueCount, queueProps.data());

    for (mPresentQueueIndex = 0; mPresentQueueIndex < queueCount; mPresentQueueIndex++)
    {
        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(gDevice->mPhysicalDevice, mPresentQueueIndex,
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
    vkGetDeviceQueue(gDevice->GetDevice(), mPresentQueueIndex, 0, &mPresentQueue);

    return true;
}

bool Backbuffer::SelectSurfaceFormat(const BackbufferDesc& desc)
{
    // Surface format selection
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gDevice->mPhysicalDevice, mSurface, &formatCount, nullptr);
    if (formatCount == 0)
    {
        LOGE("No surface formats to choose from.");
        return false;
    }

    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(gDevice->mPhysicalDevice, mSurface,
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
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(gDevice->mPhysicalDevice, mSurface,
                                                                &presentModeCount, nullptr);
    RETURN_FALSE_IF_FAILED(result, "Failed to acquire surface's present mode count");
    if ((presentModeCount == 0) || (presentModeCount == UINT32_MAX))
    {
        LOGE("Failed to get present mode count for currently acquired surface");
        return false;
    }

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(gDevice->mPhysicalDevice, mSurface,
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
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gDevice->mPhysicalDevice, mSurface, &mSurfCaps);
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
    chainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    chainInfo.preTransform = mSurfCaps.currentTransform;
    chainInfo.imageArrayLayers = 1;
    chainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    chainInfo.presentMode = mPresentMode;
    chainInfo.oldSwapchain = VK_NULL_HANDLE;
    chainInfo.clipped = VK_TRUE;
    chainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    VkResult result = vkCreateSwapchainKHR(gDevice->GetDevice(), &chainInfo, nullptr, &mSwapchain);
    RETURN_FALSE_IF_FAILED(result, "Failed to create swapchain");
    return true;
}

bool Backbuffer::AllocateImageViews()
{
    uint32_t currentBufferCount = 0;
    VkResult result = vkGetSwapchainImagesKHR(gDevice->GetDevice(), mSwapchain,
                                              &currentBufferCount, nullptr);
    RETURN_FALSE_IF_FAILED(result, "Failed to acquire Swapchain image count");

    if (currentBufferCount != mBufferCount)
    {
        LOGW("Allocated " << currentBufferCount << " images instead of " << mBufferCount);
        mBufferCount = currentBufferCount;
    }

    mImages.resize(mBufferCount);

    std::vector<VkImage> images(mBufferCount);
    result = vkGetSwapchainImagesKHR(gDevice->GetDevice(), mSwapchain,
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

    // we need image views to attach them to Render Targets later on
    VkImageViewCreateInfo ivInfo;
    for (uint32_t i = 0; i < mBufferCount; ++i)
    {
        ZERO_MEMORY(ivInfo);
        ivInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivInfo.image = mImages[i].image;
        ivInfo.format = mFormat;
        ivInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivInfo.components = {
            // order of variables in VkComponentMapping is r, g, b, a
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A,
        };
        ivInfo.subresourceRange = mSubresourceRange;
        result = vkCreateImageView(gDevice->GetDevice(), &ivInfo, nullptr, &mImages[i].view);
        RETURN_FALSE_IF_FAILED(result, "Failed to generate Image View from Swapchain image");

        mImages[i].currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    mDefaultLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    mFromSwapchain = true;

    return true;
}

bool Backbuffer::CreateImageAcquireFences()
{
    VkFenceCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkResult result = vkCreateFence(gDevice->GetDevice(), &info, nullptr, &mImageAcquireFence);
    RETURN_FALSE_IF_FAILED(result, "Failed to create fences for next Image acquisition");

    return true;
}

bool Backbuffer::AcquireNextImage()
{
    VkResult result = vkAcquireNextImageKHR(gDevice->GetDevice(), mSwapchain, UINT64_MAX,
                                            VK_NULL_HANDLE, mImageAcquireFence, &mCurrentBuffer);
    RETURN_FALSE_IF_FAILED(result, "Failed to preacquire next image for presenting");

    do
    {
        result = vkWaitForFences(gDevice->GetDevice(), 1, &mImageAcquireFence, VK_TRUE, 0);
    } while (result != VK_SUCCESS);


    result = vkResetFences(gDevice->GetDevice(), 1, &mImageAcquireFence);
    RETURN_FALSE_IF_FAILED(result, "Error while resetting Image acquisition Fence");

    return true;
}

bool Backbuffer::Init(const BackbufferDesc& desc)
{
    mInstancePtr = desc.instancePtr;
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
    if (!CreateImageAcquireFences()) return false;

    LOGI("Backbuffer initialized successfully");
    return true;
}

bool Backbuffer::Present()
{
    VkResult result = VK_SUCCESS;

    VkPresentInfoKHR presentInfo;
    ZERO_MEMORY(presentInfo);
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mSwapchain;
    presentInfo.pImageIndices = &mCurrentBuffer;
    presentInfo.pResults = &result;
    vkQueuePresentKHR(mPresentQueue, &presentInfo);
    RETURN_FALSE_IF_FAILED(result, "Failed to present rendered image on screen");

    return true;
}

} // namespace Renderer
} // namespace ABench
