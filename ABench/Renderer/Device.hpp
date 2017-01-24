#pragma once

#include "Instance.hpp"
#include "SemaphoreManager.hpp"
#include "CommandBuffer.hpp"

namespace ABench {
namespace Renderer {

class CommandBuffer;

class Device
{
    friend class Backbuffer;
    friend class Pipeline;
    friend class Buffer; // TODO remove

    SemaphoreManager* mSemaphores;

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

    __forceinline VkDevice GetDevice() const
    {
        return mDevice;
    }

    __forceinline VkCommandPool GetCommandPool() const
    {
        return mCommandPool;
    }
};

} // namespace Renderer
} // namespace ABench
