#pragma once

#include "Device.hpp"

namespace ABench {
namespace Renderer {

struct BufferDesc
{
    void* data;
    VkDeviceSize dataSize;
    VkBufferUsageFlags usage;
};

class Buffer
{
    friend class CommandBuffer;

    const Device* mDevicePtr;

    VkBuffer mBuffer;
    VkDeviceMemory mBufferMemory;
    VkDeviceSize mBufferSize;

public:
    Buffer(const Device* device);
    ~Buffer();

    bool Init(const BufferDesc& desc);
};

} // namespace Renderer
} // namespace ABench
