#include "../PCH.hpp"
#include "Backbuffer.hpp"

#include "Util.hpp"
#include "Extensions.hpp"
#include "Translations.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

Backbuffer::Backbuffer()
    : mSurface(VK_NULL_HANDLE)
{
}

Backbuffer::~Backbuffer()
{
}

bool Backbuffer::CreateSurface(const BackbufferDesc& desc)
{
    VkWin32SurfaceCreateInfoKHR surfInfo;
    ZERO_MEMORY(surfInfo);
    surfInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfInfo.hinstance = desc.hInstance;
    surfInfo.hwnd = desc.hWnd;
    VkResult result = vkCreateWin32SurfaceKHR(desc.instance->GetVkInstance(), &surfInfo, nullptr, &mSurface);
    CHECK_VKRESULT(result, "Failed to create Vulkan Surface for Win32");

    return true;
}

bool Backbuffer::GetPresentQueue(const BackbufferDesc& desc)
{
     // TODO to be replaced by Queue Manager
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(desc.device->mPhysicalDevice, &queueCount, nullptr);
    if (queueCount == 0)
    {
        LOGE("Physical device does not have any queue family properties.");
        return false;
    }
    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(desc.device->mPhysicalDevice, &queueCount, queueProps.data());

    for (mPresentQueueIndex = 0; mPresentQueueIndex < queueCount; mPresentQueueIndex++)
    {
        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(desc.device->mPhysicalDevice, mPresentQueueIndex,
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
    vkGetDeviceQueue(desc.device->mDevice, mPresentQueueIndex, 0, &mPresentQueue);

    return true;
}

bool Backbuffer::SelectSurfaceFormat(const BackbufferDesc& desc)
{
    // Surface format selection
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(desc.device->mPhysicalDevice, mSurface, &formatCount, nullptr);
    if (formatCount == 0)
    {
        LOGE("No surface formats to choose from.");
        return false;
    }

    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(desc.device->mPhysicalDevice, mSurface,
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
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(desc.device->mPhysicalDevice, mSurface,
                                                                &presentModeCount, nullptr);
    CHECK_VKRESULT(result, "Failed to acquire surface's present mode count");
    if ((presentModeCount == 0) || (presentModeCount == UINT32_MAX)) 
    {
        LOGE("Failed to get present mode count for currently acquired surface");
        return false;
    }

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(desc.device->mPhysicalDevice, mSurface,
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
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(desc.device->mPhysicalDevice, mSurface, &mSurfCaps);
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
    VkResult result = vkCreateSwapchainKHR(desc.device->mDevice, &chainInfo, nullptr, &mSwapchain);
    CHECK_VKRESULT(result, "Failed to create swapchain");
    return true;
}

bool Backbuffer::Init(const BackbufferDesc& desc)
{
    if (!CreateSurface(desc)) return false;
    if (!GetPresentQueue(desc)) return false;
    if (!SelectSurfaceFormat(desc)) return false;
    if (!SelectPresentMode(desc)) return false;
    if (!AcquireSurfaceCaps(desc)) return false;
    SelectBufferCount(desc);
    if (!CreateSwapchain(desc)) return false;

    LOGI("Backbuffer initialized successfully");
    return true;
}

} // namespace Renderer
} // namespace ABench
