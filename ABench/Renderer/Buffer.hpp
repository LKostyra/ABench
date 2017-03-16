#pragma once

#include "Device.hpp"

namespace ABench {
namespace Renderer {

struct BufferDesc
{
    Device* devicePtr;

    void* data;
    VkDeviceSize dataSize;
    VkBufferUsageFlags usage;
};

class Buffer
{
    friend class CommandBuffer;

    Device* mDevicePtr;

    VkBuffer mBuffer;
    VkDeviceMemory mBufferMemory;
    VkDeviceSize mBufferSize;

public:
    Buffer();
    ~Buffer();

    bool Init(const BufferDesc& desc);
};

} // namespace Renderer
} // namespace ABench
