#pragma once

#include "Prerequisites.hpp"
#include "Instance.hpp"
#include "CommandBuffer.hpp"
#include "QueueManager.hpp"
#include "DescriptorAllocator.hpp"
#include "Common/Common.hpp"


namespace ABench {
namespace Renderer {

class CommandBuffer;

class Device
{
    friend class Backbuffer;
    friend class Pipeline;

    VkDevice mDevice;
    VkPhysicalDevice mPhysicalDevice;
    VkPhysicalDeviceMemoryProperties mMemoryProperties;
    DescriptorAllocator mDescriptorAllocator;
    QueueManager mQueueManager;

    VkPhysicalDevice SelectPhysicalDevice(const Instance& inst);

public:
    Device();
    ~Device();

    bool Init(const Instance& inst);

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

    ABENCH_INLINE DescriptorAllocator& GetDescriptorAllocator()
    {
        return mDescriptorAllocator;
    }
};

} // namespace Renderer
} // namespace ABench
