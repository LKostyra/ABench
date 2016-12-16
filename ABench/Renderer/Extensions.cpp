#include "../PCH.hpp"
#include "Extensions.hpp"


/**
 * Acquire a function pointer from Vulkan library.
 *
 * The macro uses Library object to retrieve a pointer to specified function.
 * All such functions should be retrieved only once per program lifetime.
 */
#ifndef VK_GET_LIBPROC
#define VK_GET_LIBPROC(lib, x) do { \
    if (!(x = (PFN_##x)lib.GetSymbol(#x))) \
        allExtensionsAvailable = false; \
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
        LOG_ERROR("Unable to retrieve Device function " #x); \
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
    VK_GET_INSTANCEPROC(instance, vkGetPhysicalDeviceSurfaceSupportKHR);
    VK_GET_INSTANCEPROC(instance, vkGetDeviceProcAddr);
    VK_GET_INSTANCEPROC(instance, vkCreateDevice);
    VK_GET_INSTANCEPROC(instance, vkDestroyDevice);
    VK_GET_INSTANCEPROC(instance, vkCreateWin32SurfaceKHR);
    VK_GET_INSTANCEPROC(instance, vkDestroySurfaceKHR);

    return allExtensionsAvailable;
}


bool InitDeviceExtensions(const VkDevice& device)
{
    bool allExtensionsAvailable = true;



    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace ABench
