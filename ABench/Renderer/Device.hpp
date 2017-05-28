#pragma once

#include "Prerequisites.hpp"
#include "Instance.hpp"
#include "CommandBuffer.hpp"
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
    VkQueue mGraphicsQueue;
    uint32_t mGraphicsQueueIndex;
    VkCommandPool mCommandPool;
    DescriptorAllocator mDescriptorAllocator;

    VkPhysicalDevice SelectPhysicalDevice(const Instance& inst);

public:
    Device();
    ~Device();

    bool Init(const Instance& inst);

    uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkFlags properties) const;
    void WaitForGPU() const;

    bool Execute(CommandBuffer* cmd, VkFence waitFence = VK_NULL_HANDLE) const;

    ABENCH_INLINE VkDevice GetDevice() const
    {
        return mDevice;
    }

    ABENCH_INLINE VkCommandPool GetCommandPool() const
    {
        return mCommandPool;
    }

    ABENCH_INLINE DescriptorAllocator& GetDescriptorAllocator()
    {
        return mDescriptorAllocator;
    }
};

} // namespace Renderer
} // namespace ABench
