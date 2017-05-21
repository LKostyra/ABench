#include "PCH.hpp"
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
    x = (PFN_##x)lib.GetSymbol(#x); \
    if (!x) \
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
PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR = VK_NULL_HANDLE;

#ifdef WIN32
PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = VK_NULL_HANDLE;
#elif defined(__linux__) | defined(__LINUX__)
PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR = VK_NULL_HANDLE;
#else
#error "Target platform not supported"
#endif

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
    VK_GET_INSTANCEPROC(instance, vkDestroySurfaceKHR);

#ifdef WIN32
    VK_GET_INSTANCEPROC(instance, vkCreateWin32SurfaceKHR);
#elif defined(__linux__) | defined(__LINUX__)
    VK_GET_INSTANCEPROC(instance, vkCreateXcbSurfaceKHR);
#else
#error "Target platform not supported"
#endif

    return allExtensionsAvailable;
}


// Swapchain
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = VK_NULL_HANDLE;
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = VK_NULL_HANDLE;
PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR = VK_NULL_HANDLE;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = VK_NULL_HANDLE;
PFN_vkQueuePresentKHR vkQueuePresentKHR = VK_NULL_HANDLE;

// Queues
PFN_vkGetDeviceQueue vkGetDeviceQueue = VK_NULL_HANDLE;
PFN_vkQueueSubmit vkQueueSubmit = VK_NULL_HANDLE;
PFN_vkQueueWaitIdle vkQueueWaitIdle = VK_NULL_HANDLE;

// Buffers and Images
PFN_vkAllocateMemory vkAllocateMemory = VK_NULL_HANDLE;
PFN_vkBindBufferMemory vkBindBufferMemory = VK_NULL_HANDLE;
PFN_vkBindImageMemory vkBindImageMemory = VK_NULL_HANDLE;
PFN_vkCreateBuffer vkCreateBuffer = VK_NULL_HANDLE;
PFN_vkCreateImage vkCreateImage = VK_NULL_HANDLE;
PFN_vkCreateImageView vkCreateImageView = VK_NULL_HANDLE;
PFN_vkDestroyBuffer vkDestroyBuffer = VK_NULL_HANDLE;
PFN_vkDestroyImage vkDestroyImage = VK_NULL_HANDLE;
PFN_vkDestroyImageView vkDestroyImageView = VK_NULL_HANDLE;
PFN_vkFreeMemory vkFreeMemory = VK_NULL_HANDLE;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements = VK_NULL_HANDLE;
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements = VK_NULL_HANDLE;
PFN_vkMapMemory vkMapMemory = VK_NULL_HANDLE;
PFN_vkUnmapMemory vkUnmapMemory = VK_NULL_HANDLE;

// Synchronization
PFN_vkCreateFence vkCreateFence = VK_NULL_HANDLE;
PFN_vkCreateSemaphore vkCreateSemaphore = VK_NULL_HANDLE;
PFN_vkDestroyFence vkDestroyFence = VK_NULL_HANDLE;
PFN_vkDestroySemaphore vkDestroySemaphore = VK_NULL_HANDLE;
PFN_vkResetFences vkResetFences = VK_NULL_HANDLE;
PFN_vkWaitForFences vkWaitForFences = VK_NULL_HANDLE;

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

// Pipeline management
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines = VK_NULL_HANDLE;
PFN_vkCreatePipelineCache vkCreatePipelineCache = VK_NULL_HANDLE;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout = VK_NULL_HANDLE;
PFN_vkDestroyPipeline vkDestroyPipeline = VK_NULL_HANDLE;
PFN_vkDestroyPipelineCache vkDestroyPipelineCache = VK_NULL_HANDLE;
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout = VK_NULL_HANDLE;

// Shader
PFN_vkCreateShaderModule vkCreateShaderModule = VK_NULL_HANDLE;
PFN_vkDestroyShaderModule vkDestroyShaderModule = VK_NULL_HANDLE;

// Descriptors
PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets = VK_NULL_HANDLE;
PFN_vkCreateDescriptorPool vkCreateDescriptorPool = VK_NULL_HANDLE;
PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout = VK_NULL_HANDLE;
PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool = VK_NULL_HANDLE;
PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout = VK_NULL_HANDLE;
PFN_vkFreeDescriptorSets vkFreeDescriptorSets = VK_NULL_HANDLE;
PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets = VK_NULL_HANDLE;

// Commands
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass = VK_NULL_HANDLE;
PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets = VK_NULL_HANDLE;
PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer = VK_NULL_HANDLE;
PFN_vkCmdBindPipeline vkCmdBindPipeline = VK_NULL_HANDLE;
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers = VK_NULL_HANDLE;
PFN_vkCmdClearAttachments vkCmdClearAttachments = VK_NULL_HANDLE;
PFN_vkCmdCopyBuffer vkCmdCopyBuffer = VK_NULL_HANDLE;
PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage = VK_NULL_HANDLE;
PFN_vkCmdDraw vkCmdDraw = VK_NULL_HANDLE;
PFN_vkCmdDrawIndexed vkCmdDrawIndexed = VK_NULL_HANDLE;
PFN_vkCmdEndRenderPass vkCmdEndRenderPass = VK_NULL_HANDLE;
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier = VK_NULL_HANDLE;
PFN_vkCmdSetScissor vkCmdSetScissor = VK_NULL_HANDLE;
PFN_vkCmdSetViewport vkCmdSetViewport = VK_NULL_HANDLE;

bool InitDeviceExtensions(const VkDevice& device)
{
    bool allExtensionsAvailable = true;

    // Swapchain
    VK_GET_DEVICEPROC(device, vkAcquireNextImageKHR);
    VK_GET_DEVICEPROC(device, vkCreateSwapchainKHR);
    VK_GET_DEVICEPROC(device, vkDestroySwapchainKHR);
    VK_GET_DEVICEPROC(device, vkGetSwapchainImagesKHR);
    VK_GET_DEVICEPROC(device, vkQueuePresentKHR);

    // Queues
    VK_GET_DEVICEPROC(device, vkGetDeviceQueue);
    VK_GET_DEVICEPROC(device, vkQueueSubmit);
    VK_GET_DEVICEPROC(device, vkQueueWaitIdle);

    // Buffers and Images
    VK_GET_DEVICEPROC(device, vkAllocateMemory);
    VK_GET_DEVICEPROC(device, vkBindBufferMemory);
    VK_GET_DEVICEPROC(device, vkBindImageMemory);
    VK_GET_DEVICEPROC(device, vkCreateBuffer);
    VK_GET_DEVICEPROC(device, vkCreateImage);
    VK_GET_DEVICEPROC(device, vkCreateImageView);
    VK_GET_DEVICEPROC(device, vkDestroyBuffer);
    VK_GET_DEVICEPROC(device, vkDestroyImage);
    VK_GET_DEVICEPROC(device, vkDestroyImageView);
    VK_GET_DEVICEPROC(device, vkFreeMemory);
    VK_GET_DEVICEPROC(device, vkGetBufferMemoryRequirements);
    VK_GET_DEVICEPROC(device, vkGetImageMemoryRequirements);
    VK_GET_DEVICEPROC(device, vkMapMemory);
    VK_GET_DEVICEPROC(device, vkUnmapMemory);

    // Synchronization
    VK_GET_DEVICEPROC(device, vkCreateFence);
    VK_GET_DEVICEPROC(device, vkCreateSemaphore);
    VK_GET_DEVICEPROC(device, vkDestroyFence);
    VK_GET_DEVICEPROC(device, vkDestroySemaphore);
    VK_GET_DEVICEPROC(device, vkResetFences);
    VK_GET_DEVICEPROC(device, vkWaitForFences);

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

    // Pipeline management
    VK_GET_DEVICEPROC(device, vkCreateGraphicsPipelines);
    VK_GET_DEVICEPROC(device, vkCreatePipelineCache);
    VK_GET_DEVICEPROC(device, vkCreatePipelineLayout);
    VK_GET_DEVICEPROC(device, vkDestroyPipeline);
    VK_GET_DEVICEPROC(device, vkDestroyPipelineCache);
    VK_GET_DEVICEPROC(device, vkDestroyPipelineLayout);

    // Shader
    VK_GET_DEVICEPROC(device, vkCreateShaderModule);
    VK_GET_DEVICEPROC(device, vkDestroyShaderModule);

    // Descriptors
    VK_GET_DEVICEPROC(device, vkAllocateDescriptorSets);
    VK_GET_DEVICEPROC(device, vkCreateDescriptorPool);
    VK_GET_DEVICEPROC(device, vkCreateDescriptorSetLayout);
    VK_GET_DEVICEPROC(device, vkDestroyDescriptorPool);
    VK_GET_DEVICEPROC(device, vkDestroyDescriptorSetLayout);
    VK_GET_DEVICEPROC(device, vkFreeDescriptorSets);
    VK_GET_DEVICEPROC(device, vkUpdateDescriptorSets);

    // Commands
    VK_GET_DEVICEPROC(device, vkCmdBeginRenderPass);
    VK_GET_DEVICEPROC(device, vkCmdBindDescriptorSets);
    VK_GET_DEVICEPROC(device, vkCmdBindIndexBuffer);
    VK_GET_DEVICEPROC(device, vkCmdBindPipeline);
    VK_GET_DEVICEPROC(device, vkCmdBindVertexBuffers);
    VK_GET_DEVICEPROC(device, vkCmdClearAttachments);
    VK_GET_DEVICEPROC(device, vkCmdCopyBuffer);
    VK_GET_DEVICEPROC(device, vkCmdCopyBufferToImage);
    VK_GET_DEVICEPROC(device, vkCmdDraw);
    VK_GET_DEVICEPROC(device, vkCmdDrawIndexed);
    VK_GET_DEVICEPROC(device, vkCmdEndRenderPass);
    VK_GET_DEVICEPROC(device, vkCmdPipelineBarrier);
    VK_GET_DEVICEPROC(device, vkCmdSetScissor);
    VK_GET_DEVICEPROC(device, vkCmdSetViewport);

    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace ABench
