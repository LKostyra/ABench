#include "../PCH.hpp"
#include "Backbuffer.hpp"

#include "Util.hpp"
#include "Extensions.hpp"

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

bool Backbuffer::Init(const Instance& inst, const BackbufferDesc& desc)
{
    VkWin32SurfaceCreateInfoKHR surfInfo;
    ZERO_MEMORY(surfInfo);
    surfInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfInfo.hinstance = desc.hInstance;
    surfInfo.hwnd = desc.hWnd;
    VkResult result = vkCreateWin32SurfaceKHR(inst.GetVkInstance(), &surfInfo, nullptr, &mSurface);
    CHECK_VKRESULT(result, "Failed to create Vulkan Surface for Win32");

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

    return true;
}

} // namespace Renderer
} // namespace ABench
