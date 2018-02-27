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
    bool Execute(DeviceQueueType queueType, CommandBuffer* cmd) const;
    bool Execute(DeviceQueueType queueType, CommandBuffer* cmd, uint32_t waitSemaphoresCount,
                 const VkPipelineStageFlags* waitFlags, const VkSemaphore* waitSemaphores,
                 VkSemaphore signalSemaphore, VkFence waitFence) const;

    ABENCH_INLINE VkDevice GetDevice() const
    {
        return mDevice;
    }

    ABENCH_INLINE VkCommandPool GetCommandPool(DeviceQueueType queueType) const
    {
        return mQueueManager.GetCommandPool(queueType);
    }

    ABENCH_INLINE uint32_t GetQueueIndex(DeviceQueueType queueType) const
    {
        return mQueueManager.GetQueueIndex(queueType);
    }

    ABENCH_INLINE uint32_t GetQueueCount() const
    {
        return mQueueManager.GetQueueCount();
    }

    ABENCH_INLINE const uint32_t* GetQueueIndices() const
    {
        return mQueueManager.GetQueueIndices();
    }
};

using DevicePtr = std::shared_ptr<Device>;

} // namespace Renderer
} // namespace ABench
