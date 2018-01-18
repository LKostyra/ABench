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
    bool concurrent;

    BufferDesc()
        : data(nullptr)
        , dataSize(0)
        , usage(0)
        , type(BufferType::Static)
        , concurrent(false)
    {
    }
};

class Buffer
{
    friend class CommandBuffer;

    DevicePtr mDevice;
    BufferType mType;
    VkBuffer mBuffer;
    VkDeviceMemory mBufferMemory;
    VkDeviceSize mBufferSize;

public:
    Buffer();
    ~Buffer();

    bool Init(const DevicePtr& device, const BufferDesc& desc);
    bool Write(const void* data, size_t size, size_t offset = 0);
    void Free();
    void Dump(); // DEBUGGING FUNCTION ONLY

    ABENCH_INLINE VkBuffer GetBuffer() const
    {
        return mBuffer;
    }

    ABENCH_INLINE VkDeviceMemory GetBufferMemory() const
    {
        return mBufferMemory;
    }

    ABENCH_INLINE VkDeviceSize GetSize() const
    {
        return mBufferSize;
    }
};

using BufferPtr = std::shared_ptr<Buffer>;

} // namespace Renderer
} // namespace ABench
