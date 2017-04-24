#pragma once

#include "Device.hpp"


namespace ABench {
namespace Renderer {

class RingBuffer
{
    Device* mDevicePtr;

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
    bool Init(Device* devicePtr, const VkDeviceSize bufferSize);

    /**
     * Write data to Ring Buffer. Returns offset at which data was allocated.
     */
    uint32_t Write(const void* data, size_t dataSize);

    /**
     * Waits for given fence until last frame finishes rendering. Then,
     * advances the offsets and marks the beginning of next frame.
     *
     * TODO this needs reworking - right now this function will be our
     * bottleneck. Rendering multiple frames at once won't work this way.
     */
    bool MarkFinishedFrame(VkFence waitFence);


    ABENCH_INLINE VkBuffer GetVkBuffer() const
    {
        return mBuffer;
    }
};

} // namespace Renderer
} // namespace ABench
