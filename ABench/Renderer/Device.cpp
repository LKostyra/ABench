#include "../PCH.hpp"
#include "Device.hpp"

#include "Util.hpp"
#include "Extensions.hpp"
#include "Translations.hpp"
#include "SemaphoreManager.hpp"

#include "Common/Logger.hpp"
#include "Common/Common.hpp"


namespace ABench {
namespace Renderer {

Device::Device()
    : mSemaphores(nullptr)
    , mDevice(VK_NULL_HANDLE)
    , mPhysicalDevice(VK_NULL_HANDLE)
    , mMemoryProperties()
    , mGraphicsQueueIndex(UINT32_MAX)
    , mCommandPool(VK_NULL_HANDLE)
    , mPipelineCache(VK_NULL_HANDLE)
{
}

Device::~Device()
{
    delete mSemaphores;

    if (mPipelineCache != VK_NULL_HANDLE)
        vkDestroyPipelineCache(mDevice, mPipelineCache, nullptr);
    if (mCommandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
    if (mDevice != VK_NULL_HANDLE)
        vkDestroyDevice(mDevice, nullptr);
}

VkPhysicalDevice Device::SelectPhysicalDevice(const Instance& inst)
{
    unsigned int gpuCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(inst.mInstance, &gpuCount, nullptr);
    RETURN_FALSE_IF_FAILED(result, "Failed to acquire Physical Device count");
    if (gpuCount == 0)
    {
        LOGE("No physical devices detected");
        return false;
    }

    std::vector<VkPhysicalDevice> devices(gpuCount);
    result = vkEnumeratePhysicalDevices(inst.mInstance, &gpuCount, devices.data());
    RETURN_FALSE_IF_FAILED(result, "Failed to acquire available Physical Devices");

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

    // Select first one available (might be a subject to change later on)
    return devices[0];
}

bool Device::CreatePipelineCache()
{
    VkPipelineCacheCreateInfo cacheInfo;
    ZERO_MEMORY(cacheInfo);
    cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult result = vkCreatePipelineCache(mDevice, &cacheInfo, nullptr, &mPipelineCache);
    RETURN_FALSE_IF_FAILED(result, "Failed to create Pipeline Cache");

    return true;
}

bool Device::Init(const Instance& inst)
{
    mPhysicalDevice = SelectPhysicalDevice(inst);

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

    const char* enabledLayers[] = {
        "VK_LAYER_LUNARG_standard_validation" // for debugging
    };

    VkDeviceCreateInfo devInfo;
    ZERO_MEMORY(devInfo);
    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.queueCreateInfoCount = 1;
    devInfo.pQueueCreateInfos = &queueInfo;
    devInfo.enabledExtensionCount = 1;
    devInfo.ppEnabledExtensionNames = enabledExtensions;
    if (inst.IsDebuggingEnabled())
    {
        devInfo.enabledLayerCount = 1;
        devInfo.ppEnabledLayerNames = enabledLayers;
    }

    VkResult result = vkCreateDevice(mPhysicalDevice, &devInfo, nullptr, &mDevice);
    RETURN_FALSE_IF_FAILED(result, "Failed to create Vulkan Device");

    if (!InitDeviceExtensions(mDevice))
    {
        LOGE("Failed to initailize needed device extensions");
        return false;
    }

    // extract queue (might be useful later on when ex. submitting commands)
    vkGetDeviceQueue(mDevice, mGraphicsQueueIndex, 0, &mGraphicsQueue);

    mSemaphores = new SemaphoreManager(this);
    mSemaphores->Init();

    VkCommandPoolCreateInfo poolInfo;
    ZERO_MEMORY(poolInfo);
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = mGraphicsQueueIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    result = vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool);
    RETURN_FALSE_IF_FAILED(result, "Failed to create main Command Pool");

    LOGI("Vulkan Device initialized successfully");
    return true;
}

uint32_t Device::GetMemoryTypeIndex(uint32_t typeBits, VkFlags properties) const
{
    for (uint32_t i = 0; i < mMemoryProperties.memoryTypeCount; ++i)
    {
        if (typeBits & (1 << i))
            if (mMemoryProperties.memoryTypes[i].propertyFlags & properties)
                return i;
    }

    return UINT32_MAX;
}

void Device::WaitForGPU() const
{
    vkQueueWaitIdle(mGraphicsQueue);
}

bool Device::Execute(CommandBuffer* cmd) const
{
    VkSubmitInfo submitInfo;
    ZERO_MEMORY(submitInfo);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd->mCommandBuffer;
    submitInfo.signalSemaphoreCount = 1; // TODO semaphore management sucks, to redo
    submitInfo.pSignalSemaphores = &mSemaphores->mRenderSemaphore;
    VkResult result = vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    RETURN_FALSE_IF_FAILED(result, "Failed to submit graphics operation");

    return true;
}

} // namespace Renderer
} // namespace ABench
