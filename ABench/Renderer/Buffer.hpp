#pragma once

#include "Prerequisites.hpp"
#include "Device.hpp"
#include "Common/Common.hpp"


namespace ABench {
namespace Renderer {

enum class BufferType: unsigned short
{
    Static = 1, // Device-allocated, preinitialized
    Dynamic, // Host-allocated, can be modified. Synchronization must be ensured.
};

struct BufferDesc
{
    const void* data;
    VkDeviceSize dataSize;
    VkBufferUsageFlags usage;
    BufferType type;

    BufferDesc()
        : data(nullptr)
        , dataSize(0)
        , usage(0)
        , type(BufferType::Static)
    {
    }
};

class Buffer
{
    friend class CommandBuffer;

    BufferType mType;
    VkBuffer mBuffer;
    VkDeviceMemory mBufferMemory;
    VkDeviceSize mBufferSize;

public:
    Buffer();
    ~Buffer();

    bool Init(const BufferDesc& desc);
    void Free();

    ABENCH_INLINE VkBuffer GetVkBuffer() const
    {
        return mBuffer;
    }

    ABENCH_INLINE VkDeviceSize GetSize() const
    {
        return mBufferSize;
    }

    bool Write(const void* data, size_t size, size_t offset = 0);
};

} // namespace Renderer
} // namespace ABench
