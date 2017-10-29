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
    , mGraphicsQueue(VK_NULL_HANDLE)
    , mGraphicsCommandPool(VK_NULL_HANDLE)
    , mComputeCommandPool(VK_NULL_HANDLE)
    , mDescriptorAllocator()
    , mQueueManager()
{
}

Device::~Device()
{
    mDescriptorAllocator.Release();

    if (mGraphicsCommandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(mDevice, mGraphicsCommandPool, nullptr);
    if (mDevice != VK_NULL_HANDLE)
        vkDestroyDevice(mDevice, nullptr);
}

VkPhysicalDevice Device::SelectPhysicalDevice(const Instance& inst)
{
    unsigned int gpuCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(inst.mInstance, &gpuCount, nullptr);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to acquire Physical Device count");
    if (gpuCount == 0)
    {
        LOGE("No physical devices detected");
        return VK_NULL_HANDLE;
    }

    std::vector<VkPhysicalDevice> devices(gpuCount);
    result = vkEnumeratePhysicalDevices(inst.mInstance, &gpuCount, devices.data());
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

    // Select first one available (might be a subject to change later on)
    return devices[0];
}

bool Device::Init(const Instance& inst)
{
    mPhysicalDevice = SelectPhysicalDevice(inst);

    // Memory properties (for further use)
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mMemoryProperties);

    if (!mQueueManager.Init(mPhysicalDevice))
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

    VkDeviceCreateInfo devInfo;
    ZERO_MEMORY(devInfo);
    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.queueCreateInfoCount = static_cast<uint32_t>(mQueueManager.GetQueueCreateInfos().size());
    devInfo.pQueueCreateInfos = mQueueManager.GetQueueCreateInfos().data();
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
    vkGetDeviceQueue(mDevice, mQueueManager.GetGraphicsQueueIndex(), 0, &mGraphicsQueue);


    // Command Pool for Command Buffers
    VkCommandPoolCreateInfo poolInfo;
    ZERO_MEMORY(poolInfo);
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = mQueueManager.GetGraphicsQueueIndex();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    result = vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mGraphicsCommandPool);
    RETURN_FALSE_IF_FAILED(result, "Failed to create main Command Pool");


    // Descriptor Allocator
    DescriptorAllocatorDesc daDesc;
    ZERO_MEMORY(daDesc);
    daDesc.limits[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] = 2;
    daDesc.limits[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] = 2;
    daDesc.limits[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] = 1000;
    if (!mDescriptorAllocator.Init(mDevice, daDesc))
        return false;

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

bool Device::ExecuteGraphics(CommandBuffer* cmd, VkFence waitFence) const
{
    VkSubmitInfo submitInfo;
    ZERO_MEMORY(submitInfo);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd->mCommandBuffer;
    VkResult result = vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, waitFence);
    RETURN_FALSE_IF_FAILED(result, "Failed to submit graphics operation");

    return true;
}

} // namespace Renderer
} // namespace ABench
