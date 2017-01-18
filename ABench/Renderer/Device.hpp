#pragma once

#include "Instance.hpp"
#include "SemaphoreManager.hpp"
#include "CommandBuffer.hpp"

namespace ABench {
namespace Renderer {

class Device
{
    friend class Backbuffer;
    friend class SemaphoreManager;
    friend class RenderPass;
    friend class Framebuffer;
    friend class CommandBuffer;
    friend class Buffer;

    SemaphoreManager* mSemaphores;

    VkDevice mDevice;
    VkPhysicalDevice mPhysicalDevice;
    VkPhysicalDeviceMemoryProperties mMemoryProperties;
    VkQueue mGraphicsQueue;
    uint32_t mGraphicsQueueIndex;
    VkCommandPool mCommandPool;

    VkPhysicalDevice SelectPhysicalDevice(const std::vector<VkPhysicalDevice>& devices);

public:
    Device();
    ~Device();

    bool Init(const Instance& inst);

    uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkFlags properties) const;
    void WaitForGPU() const;

    bool Execute(CommandBuffer* cmd) const;
};

} // namespace Renderer
} // namespace ABench
