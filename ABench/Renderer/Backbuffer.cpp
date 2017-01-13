#include "../PCH.hpp"
#include "Backbuffer.hpp"

#include "Util.hpp"
#include "Extensions.hpp"
#include "Translations.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

Backbuffer::Backbuffer(const Instance* instance, const Device* device)
    : mInstancePtr(instance)
    , mDevicePtr(device)
    , mSurface(VK_NULL_HANDLE)
{
}

Backbuffer::~Backbuffer()
{
    if (mPresentCommandPool)
        vkDestroyCommandPool(mDevicePtr->mDevice, mPresentCommandPool, nullptr);
}

bool Backbuffer::CreateSurface(const BackbufferDesc& desc)
{
    VkWin32SurfaceCreateInfoKHR surfInfo;
    ZERO_MEMORY(surfInfo);
    surfInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfInfo.hinstance = desc.hInstance;
    surfInfo.hwnd = desc.hWnd;
    VkResult result = vkCreateWin32SurfaceKHR(mInstancePtr->GetVkInstance(), &surfInfo, nullptr, &mSurface);
    CHECK_VKRESULT(result, "Failed to create Vulkan Surface for Win32");

    return true;
}

bool Backbuffer::GetPresentQueue(const BackbufferDesc& desc)
{
     // TODO to be replaced by Queue Manager
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(mDevicePtr->mPhysicalDevice, &queueCount, nullptr);
    if (queueCount == 0)
    {
        LOGE("Physical device does not have any queue family properties.");
        return false;
    }
    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(mDevicePtr->mPhysicalDevice, &queueCount, queueProps.data());

    for (mPresentQueueIndex = 0; mPresentQueueIndex < queueCount; mPresentQueueIndex++)
    {
        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(mDevicePtr->mPhysicalDevice, mPresentQueueIndex,
                                             mSurface, &presentSupport);
        if (presentSupport)
            break;
    }

    if (mPresentQueueIndex == queueCount)
    {
        LOGE("No queue with present support!");
        return false;
    }

    LOGD("Selected queue with Present support #" << mPresentQueueIndex);
    vkGetDeviceQueue(mDevicePtr->mDevice, mPresentQueueIndex, 0, &mPresentQueue);

    return true;
}

bool Backbuffer::SelectSurfaceFormat(const BackbufferDesc& desc)
{
    // Surface format selection
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(mDevicePtr->mPhysicalDevice, mSurface, &formatCount, nullptr);
    if (formatCount == 0)
    {
        LOGE("No surface formats to choose from.");
        return false;
    }

    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(mDevicePtr->mPhysicalDevice, mSurface,
                                                           &formatCount, surfaceFormats.data());
    CHECK_VKRESULT(result, "Failed to acquire physical device surface formats");

    uint32_t formatIndex = 0;
    if ((formatCount > 1) || (surfaceFormats[0].format != VK_FORMAT_UNDEFINED))
        for (formatIndex = 0; formatIndex < formatCount; ++formatIndex)
            if (surfaceFormats[formatIndex].format == desc.requestedFormat)
                break;

    if (formatIndex == formatCount)
    {
        LOGE("Requested surface format " << desc.requestedFormat << " ("
          << TranslateVkFormatToString(desc.requestedFormat) << ") is not supported by device.");
        LOGD("Available surface formats:");
        for (auto format: surfaceFormats)
            LOGD("  " << format.format << " (" << TranslateVkFormatToString(format.format) << ")");
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
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(mDevicePtr->mPhysicalDevice, mSurface,
                                                                &presentModeCount, nullptr);
    CHECK_VKRESULT(result, "Failed to acquire surface's present mode count");
    if ((presentModeCount == 0) || (presentModeCount == UINT32_MAX)) 
    {
        LOGE("Failed to get present mode count for currently acquired surface");
        return false;
    }

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(mDevicePtr->mPhysicalDevice, mSurface,
                                                       &presentModeCount, presentModes.data());
    CHECK_VKRESULT(result, "Failed to acquire surface's present modes");

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
            LOGW("Unable to select non-VSync present mode. V-Sync will stay on.")
        else if (mPresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            LOGW("Mailbox present mode was unavailable. Selecting immediate mode, you may experience tearing.")
    }

    return true;
}

bool Backbuffer::AcquireSurfaceCaps(const BackbufferDesc& desc)
{
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevicePtr->mPhysicalDevice, mSurface, &mSurfCaps);
    CHECK_VKRESULT(result, "Failed to extract surface's capabilities");
    return true;
}

void Backbuffer::SelectBufferCount(const BackbufferDesc& desc)
{
    mBufferCount = desc.bufferCount;
    if (mBufferCount > mSurfCaps.maxImageCount)
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
    VkResult result = vkCreateSwapchainKHR(mDevicePtr->mDevice, &chainInfo, nullptr, &mSwapchain);
    CHECK_VKRESULT(result, "Failed to create swapchain");
    return true;
}

bool Backbuffer::AllocateImageViews(const BackbufferDesc& desc)
{
    mImages.resize(mBufferCount);
    mImageViews.resize(mBufferCount);

    VkResult result = vkGetSwapchainImagesKHR(mDevicePtr->mDevice, mSwapchain,
                                              &mBufferCount, mImages.data());
    CHECK_VKRESULT(result, "Failed to acquire Swapchain images");
    LOGD(mBufferCount << " swapchain images acquired.");

    // we need image views to attach them to Render Targets later on
    VkImageViewCreateInfo ivInfo;
    for (uint32_t i = 0; i < mBufferCount; ++i)
    {
        ZERO_MEMORY(ivInfo);
        ivInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivInfo.image = mImages[i];
        ivInfo.format = mFormat;
        ivInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivInfo.components = {
            // order of variables in VkComponentMapping is r, g, b, a
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A,
        };
        ivInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ivInfo.subresourceRange.baseMipLevel = 0;
        ivInfo.subresourceRange.levelCount = 1;
        ivInfo.subresourceRange.baseArrayLayer = 0;
        ivInfo.subresourceRange.layerCount = 1;
        result = vkCreateImageView(mDevicePtr->mDevice, &ivInfo, nullptr, &mImageViews[i]);
        CHECK_VKRESULT(result, "Failed to generate Image View from Swapchain image");
    }

    return true;
}

bool Backbuffer::AllocateCommandBuffers(const BackbufferDesc& desc)
{
    // some pre-work needed to create command buffers
    VkCommandPoolCreateInfo poolInfo;
    ZERO_MEMORY(poolInfo);
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = mPresentQueueIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkResult result = vkCreateCommandPool(mDevicePtr->mDevice, &poolInfo,
                                          nullptr, &mPresentCommandPool);
    CHECK_VKRESULT(result, "Failed to create command pool for present command buffers");

    mPresentCommandBuffers.resize(mBufferCount);
    VkCommandBufferAllocateInfo cmdAllocInfo;
    ZERO_MEMORY(cmdAllocInfo);
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.commandPool = mPresentCommandPool;
    cmdAllocInfo.commandBufferCount = mBufferCount;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    result = vkAllocateCommandBuffers(mDevicePtr->mDevice, &cmdAllocInfo, mPresentCommandBuffers.data());
    CHECK_VKRESULT(result, "Failed to allocate command buffers for presenting");

    // building present command buffers
    VkImageMemoryBarrier postPresentBarrier;
    ZERO_MEMORY(postPresentBarrier);
    postPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    postPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    postPresentBarrier.srcAccessMask = 0;
    postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    postPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    postPresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    postPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    postPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    postPresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    postPresentBarrier.subresourceRange.baseMipLevel = 0;
    postPresentBarrier.subresourceRange.levelCount = 1;
    postPresentBarrier.subresourceRange.baseArrayLayer = 0;
    postPresentBarrier.subresourceRange.layerCount = 1;

    VkCommandBufferBeginInfo cmdBeginInfo;
    ZERO_MEMORY(cmdBeginInfo);
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    for (uint32_t i = 0; i < mBufferCount; ++i)
    {
        result = vkBeginCommandBuffer(mPresentCommandBuffers[i], &cmdBeginInfo);
        CHECK_VKRESULT(result, "Failed to start recording present command buffer");

        postPresentBarrier.image = mImages[i];

        vkCmdPipelineBarrier(mPresentCommandBuffers[i], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
                             0, nullptr, 0, nullptr,
                             1, &postPresentBarrier);

        result = vkEndCommandBuffer(mPresentCommandBuffers[i]);
        CHECK_VKRESULT(result, "Error during present command buffer recording");
    }

    return true;
}

bool Backbuffer::AcquireNextImage()
{
    VkResult result = vkAcquireNextImageKHR(mDevicePtr->mDevice, mSwapchain, UINT64_MAX,
                                            mDevicePtr->mSemaphores->mPresentSemaphore, VK_NULL_HANDLE,
                                            &mCurrentBufferIndex);
    CHECK_VKRESULT(result, "Failed to preacquire next image for presenting");

    VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo submitInfo;
    ZERO_MEMORY(submitInfo);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mPresentCommandBuffers[mCurrentBufferIndex];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &mDevicePtr->mSemaphores->mPresentSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &mDevicePtr->mSemaphores->mPostPresentSemaphore;
    result = vkQueueSubmit(mPresentQueue, 1, &submitInfo, VK_NULL_HANDLE);
    CHECK_VKRESULT(result, "Failed to submit post-acquire barrier operation");
    
    return true;
}

bool Backbuffer::Init(const BackbufferDesc& desc)
{
    mWidth = desc.width;
    mHeight = desc.height;

    if (!CreateSurface(desc)) return false;
    if (!GetPresentQueue(desc)) return false;
    if (!SelectSurfaceFormat(desc)) return false;
    if (!SelectPresentMode(desc)) return false;
    if (!AcquireSurfaceCaps(desc)) return false;
    SelectBufferCount(desc);
    if (!CreateSwapchain(desc)) return false;
    if (!AllocateImageViews(desc)) return false;
    if (!AllocateCommandBuffers(desc)) return false;

    // now we are initialized, prepare for upcoming render tasks
    if (!AcquireNextImage()) return false;

    LOGI("Backbuffer initialized successfully");
    return true;
}

bool Backbuffer::Present()
{
    VkResult result;

    VkPresentInfoKHR presentInfo;
    ZERO_MEMORY(presentInfo);
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mSwapchain;
    presentInfo.pImageIndices = &mCurrentBufferIndex;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &mDevicePtr->mSemaphores->mRenderSemaphore;
    presentInfo.pResults = &result;
    vkQueuePresentKHR(mPresentQueue, &presentInfo);
    CHECK_VKRESULT(result, "Failed to present rendered image on screen");

    return AcquireNextImage();
}

} // namespace Renderer
} // namespace ABench
