#include "PCH.hpp"
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
    , mPhysicalDevice(VK_NULL_HANDLE)
    , mMemoryProperties()
    , mQueueManager()
{
}

Device::~Device()
{
    // all Device-exclusive objects must be freed here, before VkDevice
    // otherwise we'll face resource leaks
    mQueueManager.Release();

    if (mDevice != VK_NULL_HANDLE)
        vkDestroyDevice(mDevice, nullptr);
}

VkPhysicalDevice Device::SelectPhysicalDevice()
{
    unsigned int gpuCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(mInstance->mInstance, &gpuCount, nullptr);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to acquire Physical Device count");
    if (gpuCount == 0)
    {
        LOGE("No physical devices detected");
        return VK_NULL_HANDLE;
    }

    std::vector<VkPhysicalDevice> devices(gpuCount);
    result = vkEnumeratePhysicalDevices(mInstance->mInstance, &gpuCount, devices.data());
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to acquire available Physical Devices");

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

    // select first available discrete GPU
    for (size_t i = 0; i < devices.size(); ++i)
    {
        vkGetPhysicalDeviceProperties(devices[i], &devProps);
        if (devProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            return devices[i];
    }

    // if not available, select integrated GPU
    for (size_t i = 0; i < devices.size(); ++i)
    {
        vkGetPhysicalDeviceProperties(devices[i], &devProps);
        if (devProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            return devices[i];
    }

    // no suitable physical device available
    return VK_NULL_HANDLE;
}

bool Device::Init(const InstancePtr& inst)
{
    mInstance = inst;
    mPhysicalDevice = SelectPhysicalDevice();

    // Memory properties (for further use)
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mMemoryProperties);

    if (!mQueueManager.Init(mPhysicalDevice, true))
    {
        LOGE("Failed to initialize Queue Manager");
        return false;
    }

    // device extensions
    const char* enabledExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    const char* enabledLayers[] = {
        "VK_LAYER_LUNARG_standard_validation" // for debugging
    };

    const QueueManager::QueueCreateInfos& queueInfos = mQueueManager.GetQueueCreateInfos();
    VkDeviceCreateInfo devInfo;
    ZERO_MEMORY(devInfo);
    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
    devInfo.pQueueCreateInfos = queueInfos.data();
    devInfo.enabledExtensionCount = 1;
    devInfo.ppEnabledExtensionNames = enabledExtensions;
    if (mInstance->IsDebuggingEnabled())
    {
        LOGW("Debug enabled - adding Vulkan Validation layer");
        devInfo.enabledLayerCount = 1;
        devInfo.ppEnabledLayerNames = enabledLayers;
    }

    VkResult result = vkCreateDevice(mPhysicalDevice, &devInfo, nullptr, &mDevice);
    RETURN_FALSE_IF_FAILED(result, "Failed to create Vulkan Device");

    // acquire per-device extensions
    if (!InitDeviceExtensions(mDevice))
    {
        LOGE("Failed to initailize needed device extensions");
        return false;
    }

    // finish initialization of Queue Manager
    if (!mQueueManager.CreateQueues(mDevice))
    {
        LOGE("Failed to acquire needed queues from Device");
        return false;
    }

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

void Device::Wait(DeviceQueueType queueType) const
{
    vkQueueWaitIdle(mQueueManager.GetQueue(queueType));
}

bool Device::Execute(DeviceQueueType queueType, CommandBuffer* cmd) const
{
    VkSubmitInfo submitInfo;
    ZERO_MEMORY(submitInfo);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd->mCommandBuffer;
    VkResult result = vkQueueSubmit(mQueueManager.GetQueue(queueType), 1, &submitInfo, VK_NULL_HANDLE);
    RETURN_FALSE_IF_FAILED(result, "Failed to execute command buffer");

    return true;
}

bool Device::Execute(DeviceQueueType queueType, CommandBuffer* cmd, VkSemaphore waitSemaphore,
                     VkSemaphore signalSemaphore, VkFence waitFence) const
{
    VkPipelineStageFlags waitFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo;
    ZERO_MEMORY(submitInfo);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd->mCommandBuffer;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &waitSemaphore;
    submitInfo.pWaitDstStageMask = waitFlags;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signalSemaphore;
    VkResult result = vkQueueSubmit(mQueueManager.GetQueue(queueType), 1, &submitInfo, waitFence);
    RETURN_FALSE_IF_FAILED(result, "Failed to execute command buffer");

    return true;
}

} // namespace Renderer
} // namespace ABench
