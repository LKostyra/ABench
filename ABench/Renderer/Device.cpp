#include "../PCH.hpp"
#include "Device.hpp"

#include "Util.hpp"
#include "Extensions.hpp"
#include "Translations.hpp"

#include "Common/Logger.hpp"
#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

Device::Device()
    : mDevice(VK_NULL_HANDLE)
{
}

Device::~Device()
{
}

bool Device::Init(const Instance& inst)
{
    const VkInstance& instance = inst.GetVkInstance();

    unsigned int gpuCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
    CHECK_VKRESULT(result, "Failed to acquire Physical Device count");
    if (gpuCount == 0)
    {
        LOGE("No physical devices detected");
        return false;
    }

    std::vector<VkPhysicalDevice> devices(gpuCount);
    result = vkEnumeratePhysicalDevices(instance, &gpuCount, devices.data());
    CHECK_VKRESULT(result, "Failed to acquire available Physical Devices");

    VkPhysicalDeviceProperties devProps;

    // Debugging-related device description printing
    LOGD(devices.size() << " physical devices detected:");
    for (unsigned int i = 0; i < devices.size(); ++i)
    {
        vkGetPhysicalDeviceProperties(devices[i], &devProps);
        LOGD("Device #" << i << " - " << devProps.deviceName);
        LOGD("  ID:         " << devProps.deviceID);
        LOGD("  Type:       " << devProps.deviceType << " ("
                              << TranslateVkPhysicalDeviceTypeToString(devProps.deviceType) << ")");
        LOGD("  Vendor ID:  " << devProps.vendorID);
        LOGD("  API ver:    " << VK_VERSION_MAJOR(devProps.apiVersion) << "."
                              << VK_VERSION_MINOR(devProps.apiVersion) << "."
                              << VK_VERSION_PATCH(devProps.apiVersion));
        LOGD("  Driver ver: " << VK_VERSION_MAJOR(devProps.driverVersion) << "."
                              << VK_VERSION_MINOR(devProps.driverVersion) << "."
                              << VK_VERSION_PATCH(devProps.driverVersion));
    }

    mPhysicalDevice = devices[0];

    // Memory properties (for further use)
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mMemoryProperties);

    // Queue properties gathering and printing
    // TODO should be replaced by Queue Manager
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueCount, nullptr);
    if (queueCount == 0)
    {
        LOGE("Physical device does not have any queue family properties.");
        return false;
    }
    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueCount, queueProps.data());

    for (uint32_t i = 0; i < queueCount; ++i)
    {
        LOGD("Queue #" << i << ":");
        LOGD("  Flags: " << std::hex << queueProps[i].queueFlags << " ("
                         << TranslateVkQueueFlagsToString(queueProps[i].queueFlags) << ")");
    }

    // go through all the queues and find one, which supports Graphics operations
    // TODO it probably should be managed outside Device
    for (mGraphicsQueueIndex = 0; mGraphicsQueueIndex < queueCount; mGraphicsQueueIndex++)
        if (queueProps[mGraphicsQueueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            break;

    if (mGraphicsQueueIndex == queueCount)
    {
        LOGE("Unable to find graphics-capable queue.");
        return false;
    }

    // use these queue informations in a device-create structure
    float queuePriorities[] = { 0.0f };
    VkDeviceQueueCreateInfo queueInfo;
    ZERO_MEMORY(queueInfo);
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = mGraphicsQueueIndex;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = queuePriorities;

    // device extensions
    const char* enabledExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkDeviceCreateInfo devInfo;
    ZERO_MEMORY(devInfo);
    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.queueCreateInfoCount = 1;
    devInfo.pQueueCreateInfos = &queueInfo;
    devInfo.enabledExtensionCount = 1;
    devInfo.ppEnabledExtensionNames = enabledExtensions;
    result = vkCreateDevice(mPhysicalDevice, &devInfo, nullptr, &mDevice);
    CHECK_VKRESULT(result, "Failed to create Vulkan Device");

    if (!InitDeviceExtensions(mDevice))
    {
        LOGE("Failed to initailize needed device extensions");
        return false;
    }

    LOGI("Vulkan Device initialized successfully");
    return true;
}

} // namespace Renderer
} // namespace ABench
