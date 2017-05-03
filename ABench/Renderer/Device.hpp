#pragma once

#include "Prerequisites.hpp"
#include "Instance.hpp"
#include "CommandBuffer.hpp"
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
    VkPipelineCache mPipelineCache;

    VkPhysicalDevice SelectPhysicalDevice(const Instance& inst);
    bool CreatePipelineCache();

public:
    Device();
    ~Device();

    bool Init(const Instance& inst);

    uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkFlags properties) const;
    void WaitForGPU() const;

    bool Execute(CommandBuffer* cmd) const;

    ABENCH_INLINE VkDevice GetDevice() const
    {
        return mDevice;
    }

    ABENCH_INLINE VkCommandPool GetCommandPool() const
    {
        return mCommandPool;
    }
};

} // namespace Renderer
} // namespace ABench
