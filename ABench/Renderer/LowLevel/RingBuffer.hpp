#pragma once

#include "Prerequisites.hpp"
#include "Device.hpp"


namespace ABench {
namespace Renderer {

class RingBuffer
{
    DevicePtr mDevice;

    VkBuffer mBuffer;
    VkDeviceMemory mBufferMemory;
    VkDeviceSize mBufferSize;
    uint32_t mCurrentOffset;
    uint32_t mStartOffset;
    char* mMemoryPointer;

public:
    RingBuffer();
    ~RingBuffer();

    /**
     * Initializes Ring Buffer. bufferSize means the buffer will allocate
     * specified size and loop when the limit will be exceeded.
     *
     * The Buffer does not loop the data around when the size limit is met.
     * Thus, huge data shouldn't be allocated this way, or the size should
     * be big enough.
     */
    bool Init(const DevicePtr& device, const VkDeviceSize bufferSize);

    /**
     * Write data to Ring Buffer. Returns offset at which data was allocated.
     */
    uint32_t Write(const void* data, size_t dataSize);

    /**
     * Advances the offsets and marks the beginning of next frame.
     */
    bool MarkFinishedFrame();

    ABENCH_INLINE VkBuffer GetVkBuffer() const
    {
        return mBuffer;
    }
};

} // namespace Renderer
} // namespace ABench
