#pragma once

#include "Instance.hpp"
#include "SemaphoreManager.hpp"

namespace ABench {
namespace Renderer {

// An object representing Vulkan's VkDevice
class Device
{
    friend class Backbuffer;
    friend class SemaphoreManager;
    friend class RenderPass;
    friend class Framebuffer;

    SemaphoreManager* mSemaphores;

    VkDevice mDevice;
    VkPhysicalDevice mPhysicalDevice;
    VkPhysicalDeviceMemoryProperties mMemoryProperties;
    uint32_t mGraphicsQueueIndex;

    VkPhysicalDevice SelectPhysicalDevice(const std::vector<VkPhysicalDevice>& devices);

public:
    Device();
    ~Device();

    bool Init(const Instance& inst);
};

} // namespace Renderer
} // namespace ABench
