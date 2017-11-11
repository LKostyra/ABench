#pragma once

#include "Prerequisites.hpp"
#include "Instance.hpp"
#include "CommandBuffer.hpp"
#include "QueueManager.hpp"
#include "Common/Common.hpp"


namespace ABench {
namespace Renderer {

class Device
{
    friend class Backbuffer;
    friend class Pipeline;

    InstancePtr mInstance;
    VkDevice mDevice;
    VkPhysicalDevice mPhysicalDevice;
    VkPhysicalDeviceMemoryProperties mMemoryProperties;
    QueueManager mQueueManager;

    VkPhysicalDevice SelectPhysicalDevice();

public:
    Device();
    ~Device();

    bool Init(const InstancePtr& inst);

    uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkFlags properties) const;

    void Wait(DeviceQueueType queueType) const;
    bool Execute(DeviceQueueType queueType, CommandBuffer* cmd, VkFence waitFence = VK_NULL_HANDLE) const;

    ABENCH_INLINE VkDevice GetDevice() const
    {
        return mDevice;
    }

    ABENCH_INLINE VkCommandPool GetCommandPool(DeviceQueueType queueType) const
    {
        return mQueueManager.GetCommandPool(queueType);
    }
};

} // namespace Renderer
} // namespace ABench
