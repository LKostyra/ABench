#include "../PCH.hpp"
#include "Extensions.hpp"

#include "Common/Logger.hpp"

/**
 * Acquire a function pointer from Vulkan library.
 *
 * The macro uses Library object to retrieve a pointer to specified function.
 * All such functions should be retrieved only once per program lifetime.
 */
#ifndef VK_GET_LIBPROC
#define VK_GET_LIBPROC(lib, x) do { \
    if (!(x = (PFN_##x)lib.GetSymbol(#x))) \
    { \
        LOGE("Unable to retrieve Library function " #x); \
        allExtensionsAvailable = false; \
    } \
} while(0)
#endif

/**
 * Acquire a function from VkDevice object.
 *
 * These functions must be acquired per-Instance. Since there will be only one Vulkan Instance,
 * each functon will probably be retrieved only once.
 */
#ifndef VK_GET_INSTANCEPROC
#define VK_GET_INSTANCEPROC(inst, x) do { \
    x = (PFN_##x)vkGetInstanceProcAddr(inst, #x); \
    if (!x) \
    { \
        LOGE("Unable to retrieve Instance function " #x); \
        allExtensionsAvailable = false; \
    } \
} while(0)
#endif

/**
 * Acquire a function from VkDevice object.
 *
 * These functions must be acquired per-Device. Each device must keep its own functions.
 */
#ifndef VK_GET_DEVICEPROC
#define VK_GET_DEVICEPROC(dev, x) do { \
    x = (PFN_##x)vkGetDeviceProcAddr(dev, #x); \
    if (!x) \
    { \
        LOGE("Unable to retrieve Device function " #x); \
        allExtensionsAvailable = false; \
    } \
} while(0)
#endif


namespace ABench {
namespace Renderer {

PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = VK_NULL_HANDLE;
PFN_vkCreateInstance vkCreateInstance = VK_NULL_HANDLE;
PFN_vkDestroyInstance vkDestroyInstance = VK_NULL_HANDLE;

bool InitLibraryExtensions(Common::Library& library)
{
    bool allExtensionsAvailable = true;

    VK_GET_LIBPROC(library, vkGetInstanceProcAddr);
    VK_GET_LIBPROC(library, vkCreateInstance);
    VK_GET_LIBPROC(library, vkDestroyInstance);

    return allExtensionsAvailable;
}


PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = VK_NULL_HANDLE;
PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = VK_NULL_HANDLE;
PFN_vkCreateDevice vkCreateDevice = VK_NULL_HANDLE;
PFN_vkDestroyDevice vkDestroyDevice = VK_NULL_HANDLE;
PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = VK_NULL_HANDLE;
PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR = VK_NULL_HANDLE;

bool InitInstanceExtensions(const VkInstance& instance)
{
    bool allExtensionsAvailable = true;

    VK_GET_INSTANCEPROC(instance, vkEnumeratePhysicalDevices);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceProperties);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceFeatures);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceMemoryProperties);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceQueueFamilyProperties);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceSurfaceFormatsKHR);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceSurfacePresentModesKHR);
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceSurfaceSupportKHR);
    VK_GET_INSTANCEPROC(instance, vkGetDeviceProcAddr);
    VK_GET_INSTANCEPROC(instance, vkCreateDevice);
    VK_GET_INSTANCEPROC(instance, vkDestroyDevice);
    VK_GET_INSTANCEPROC(instance, vkCreateWin32SurfaceKHR);
    VK_GET_INSTANCEPROC(instance, vkDestroySurfaceKHR);

    return allExtensionsAvailable;
}


// Swapchain
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = VK_NULL_HANDLE;
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = VK_NULL_HANDLE;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = VK_NULL_HANDLE;
PFN_vkQueuePresentKHR vkQueuePresentKHR = VK_NULL_HANDLE;

// Queues
PFN_vkGetDeviceQueue vkGetDeviceQueue = VK_NULL_HANDLE;
PFN_vkQueueSubmit vkQueueSubmit = VK_NULL_HANDLE;

// Images
PFN_vkCreateImageView vkCreateImageView = VK_NULL_HANDLE;

// Synchronization
PFN_vkCreateSemaphore vkCreateSemaphore = VK_NULL_HANDLE;
PFN_vkDestroySemaphore vkDestroySemaphore = VK_NULL_HANDLE;
PFN_vkQueueWaitIdle vkQueueWaitIdle = VK_NULL_HANDLE;

// Command Buffers
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = VK_NULL_HANDLE;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer = VK_NULL_HANDLE;
PFN_vkCreateCommandPool vkCreateCommandPool = VK_NULL_HANDLE;
PFN_vkDestroyCommandPool vkDestroyCommandPool = VK_NULL_HANDLE;
PFN_vkEndCommandBuffer vkEndCommandBuffer = VK_NULL_HANDLE;
PFN_vkFreeCommandBuffers vkFreeCommandBuffers = VK_NULL_HANDLE;

// Render Passes / Framebuffers
PFN_vkCreateFramebuffer vkCreateFramebuffer = VK_NULL_HANDLE;
PFN_vkCreateRenderPass vkCreateRenderPass = VK_NULL_HANDLE;
PFN_vkDestroyFramebuffer vkDestroyFramebuffer = VK_NULL_HANDLE;
PFN_vkDestroyRenderPass vkDestroyRenderPass = VK_NULL_HANDLE;

// Commands
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier = VK_NULL_HANDLE;

bool InitDeviceExtensions(const VkDevice& device)
{
    bool allExtensionsAvailable = true;

    // Swapchain
    VK_GET_DEVICEPROC(device, vkAcquireNextImageKHR);
    VK_GET_DEVICEPROC(device, vkCreateSwapchainKHR);
    VK_GET_DEVICEPROC(device, vkGetSwapchainImagesKHR);
    VK_GET_DEVICEPROC(device, vkQueuePresentKHR);

    // Queues
    VK_GET_DEVICEPROC(device, vkGetDeviceQueue);
    VK_GET_DEVICEPROC(device, vkQueueSubmit);
    VK_GET_DEVICEPROC(device, vkQueueWaitIdle);

    // Images
    VK_GET_DEVICEPROC(device, vkCreateImageView);

    // Synchronization
    VK_GET_DEVICEPROC(device, vkCreateSemaphore);
    VK_GET_DEVICEPROC(device, vkDestroySemaphore);

    // Command Buffers
    VK_GET_DEVICEPROC(device, vkAllocateCommandBuffers);
    VK_GET_DEVICEPROC(device, vkBeginCommandBuffer);
    VK_GET_DEVICEPROC(device, vkCreateCommandPool);
    VK_GET_DEVICEPROC(device, vkDestroyCommandPool);
    VK_GET_DEVICEPROC(device, vkEndCommandBuffer);
    VK_GET_DEVICEPROC(device, vkFreeCommandBuffers);

    // Render Passes / Framebuffers
    VK_GET_DEVICEPROC(device, vkCreateFramebuffer);
    VK_GET_DEVICEPROC(device, vkCreateRenderPass);
    VK_GET_DEVICEPROC(device, vkDestroyFramebuffer);
    VK_GET_DEVICEPROC(device, vkDestroyRenderPass);

    // Commands
    VK_GET_DEVICEPROC(device, vkCmdPipelineBarrier);

    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace ABench
