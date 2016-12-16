#pragma once

#include "Instance.hpp"

namespace ABench {
namespace Renderer {

// An object representing Vulkan's VkDevice
class Device
{
    friend class Backbuffer;

    VkDevice mDevice;
    VkPhysicalDevice mPhysicalDevice;
    VkPhysicalDeviceMemoryProperties mMemoryProperties;
    uint32_t mGraphicsQueueIndex;

public:
    Device();
    ~Device();

    bool Init(const Instance& inst);
};

} // namespace Renderer
} // namespace ABench
