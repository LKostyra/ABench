#pragma once

#include "Device.hpp"

namespace ABench {
namespace Renderer {

enum class BufferType: unsigned short
{
    Static = 1, // Device-allocated, preinitialized
    Dynamic, // Host-allocated, can be modified. Synchronization must be ensured.
    Volatile, // Frequently modified through Ring Buffer, for small data portions only.
};

struct BufferDesc
{
    Device* devicePtr;

    void* data;
    VkDeviceSize dataSize;
    VkBufferUsageFlags usage;
    BufferType type;

    BufferDesc()
        : devicePtr(nullptr)
        , data(nullptr)
        , dataSize(0)
        , usage(0)
        , type(BufferType::Static)
    {
    }
};

class Buffer
{
    friend class CommandBuffer;

    Device* mDevicePtr;

    BufferType mType;
    VkBuffer mBuffer;
    VkDeviceMemory mBufferMemory;
    VkDeviceSize mBufferSize;

public:
    Buffer();
    ~Buffer();

    bool Init(const BufferDesc& desc);

    ABENCH_INLINE VkBuffer GetVkBuffer() const
    {
        return mBuffer;
    }

    ABENCH_INLINE VkDeviceSize GetSize() const
    {
        return mBufferSize;
    }

    bool Write(const void* data, size_t size);
};

} // namespace Renderer
} // namespace ABench
