#include "PCH.hpp"
#include "RingBuffer.hpp"
#include "Extensions.hpp"
#include "Util.hpp"
#include "Renderer/HighLevel/Renderer.hpp"

#include "Common/Common.hpp"


namespace ABench {
namespace Renderer {

RingBuffer::RingBuffer()
    : mDevice()
    , mBuffer(VK_NULL_HANDLE)
    , mBufferMemory(VK_NULL_HANDLE)
    , mBufferSize(0)
    , mCurrentOffset(0)
    , mStartOffset(0)
    , mMemoryPointer(nullptr)
{
}

RingBuffer::~RingBuffer()
{
    if (mMemoryPointer)
        vkUnmapMemory(mDevice->GetDevice(), mBufferMemory);
    if (mBufferMemory != VK_NULL_HANDLE)
        vkFreeMemory(mDevice->GetDevice(), mBufferMemory, nullptr);
    if (mBuffer != VK_NULL_HANDLE)
        vkDestroyBuffer(mDevice->GetDevice(), mBuffer, nullptr);
}

bool RingBuffer::Init(const DevicePtr& device, VkDeviceSize bufferSize)
{
    mDevice = device;

    // create our buffer
    VkBufferCreateInfo bufInfo;
    ZERO_MEMORY(bufInfo);
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = bufferSize;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkResult result = vkCreateBuffer(mDevice->GetDevice(), &bufInfo, nullptr, &mBuffer);
    RETURN_FALSE_IF_FAILED(result, "Failed to create device buffer");

    VkMemoryRequirements deviceMemReqs;
    vkGetBufferMemoryRequirements(mDevice->GetDevice(), mBuffer, &deviceMemReqs);

    VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkMemoryAllocateInfo memInfo;
    ZERO_MEMORY(memInfo);
    memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memInfo.allocationSize = deviceMemReqs.size;
    memInfo.memoryTypeIndex = mDevice->GetMemoryTypeIndex(deviceMemReqs.memoryTypeBits, memFlags);
    result = vkAllocateMemory(mDevice->GetDevice(), &memInfo, nullptr, &mBufferMemory);
    RETURN_FALSE_IF_FAILED(result, "Failed to allocate device memory");

    result = vkBindBufferMemory(mDevice->GetDevice(), mBuffer, mBufferMemory, 0);
    RETURN_FALSE_IF_FAILED(result, "Failed to bind device memory to device buffer");

    result = vkMapMemory(mDevice->GetDevice(), mBufferMemory, 0, deviceMemReqs.size, 0, reinterpret_cast<void**>(&mMemoryPointer));
    RETURN_FALSE_IF_FAILED(result, "Failed to map Ring Buffer's memory to host");

    mBufferSize = deviceMemReqs.size;
    mCurrentOffset = mStartOffset = 0;

    return true;
}

uint32_t RingBuffer::Write(const void* data, size_t dataSize)
{
    uint32_t dataHead = mCurrentOffset;

    if (dataHead + dataSize > mBufferSize)
        dataHead = 0; // exceeded buffer's capacity, go back to front

    if (dataHead + dataSize == mStartOffset)
        return UINT32_MAX; // filled and cannot write.

    memcpy(mMemoryPointer + dataHead, data, dataSize);

    // update pointers
    // first, size is converted into multiple of 256 bytes, as required by Vulkan specification
    dataSize = (dataSize + 255) & ~255;
    mCurrentOffset = dataHead + static_cast<uint32_t>(dataSize);

    return dataHead;
}

bool RingBuffer::MarkFinishedFrame(VkFence waitFence)
{
    VkResult result = vkWaitForFences(mDevice->GetDevice(), 1, &waitFence, VK_TRUE, UINT64_MAX);
    RETURN_FALSE_IF_FAILED(result, "Failure to wait for rendering fence");

    vkResetFences(mDevice->GetDevice(), 1, &waitFence);

    mStartOffset = mCurrentOffset;
    return true;
}

} // namespace Renderer
} // namespace ABench
