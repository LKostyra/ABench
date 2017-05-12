#include "PCH.hpp"
#include "Buffer.hpp"
#include "Util.hpp"
#include "Extensions.hpp"
#include "CommandBuffer.hpp"
#include "Renderer.hpp"

#include "Common/Common.hpp"


namespace ABench {
namespace Renderer {

Buffer::Buffer()
{
}

Buffer::~Buffer()
{
    if (mBufferMemory)
        vkFreeMemory(gDevice->GetDevice(), mBufferMemory, nullptr);
    if (mBuffer)
        vkDestroyBuffer(gDevice->GetDevice(), mBuffer, nullptr);
}

bool Buffer::Init(const BufferDesc& desc)
{
    // TODO implement Ring Buffer mechanism
    if (desc.type == BufferType::Volatile)
    {
        LOGE("Volatile Buffers are not yet implemented");
        return false;
    }

    if ((desc.type == BufferType::Static) && (!desc.data || desc.dataSize == 0))
    {
        LOGE("Invalid/empty data provided for Static Buffer initialization");
        return false;
    }

    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    VkBuffer staging = VK_NULL_HANDLE;

    mBufferSize = desc.dataSize;

    // create our buffer
    VkBufferCreateInfo bufInfo;
    ZERO_MEMORY(bufInfo);
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = mBufferSize;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufInfo.usage = desc.usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkResult result = vkCreateBuffer(gDevice->GetDevice(), &bufInfo, nullptr, &mBuffer);
    RETURN_FALSE_IF_FAILED(result, "Failed to create device buffer");

    VkMemoryRequirements deviceMemReqs;
    vkGetBufferMemoryRequirements(gDevice->GetDevice(), mBuffer, &deviceMemReqs);

    VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (desc.type == BufferType::Static)
        memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VkMemoryAllocateInfo memInfo;
    ZERO_MEMORY(memInfo);
    memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memInfo.allocationSize = deviceMemReqs.size;
    memInfo.memoryTypeIndex = gDevice->GetMemoryTypeIndex(deviceMemReqs.memoryTypeBits, memFlags);
    result = vkAllocateMemory(gDevice->GetDevice(), &memInfo, nullptr, &mBufferMemory);
    RETURN_FALSE_IF_FAILED(result, "Failed to allocate device memory");

    result = vkBindBufferMemory(gDevice->GetDevice(), mBuffer, mBufferMemory, 0);
    RETURN_FALSE_IF_FAILED(result, "Failed to bind device memory to device buffer");

    if (desc.data != nullptr)
    {
        // create a staging buffer for copy operations
        bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        result = vkCreateBuffer(gDevice->GetDevice(), &bufInfo, nullptr, &staging);
        RETURN_FALSE_IF_FAILED(result, "Failed to create staging buffer");

        // get staging buffer's memory requirements
        VkMemoryRequirements stagingMemReqs;
        vkGetBufferMemoryRequirements(gDevice->GetDevice(), staging, &stagingMemReqs);

        // allocate memory for staging buffer
        memInfo.allocationSize = stagingMemReqs.size;
        memInfo.memoryTypeIndex = gDevice->GetMemoryTypeIndex(stagingMemReqs.memoryTypeBits,
                                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        result = vkAllocateMemory(gDevice->GetDevice(), &memInfo, nullptr, &stagingMemory);
        RETURN_FALSE_IF_FAILED(result, "Failed to allocate memory for staging buffer");

        // map the memory and copy data to it
        void* stagingData = nullptr;
        result = vkMapMemory(gDevice->GetDevice(), stagingMemory, 0, stagingMemReqs.size, 0, &stagingData);
        RETURN_FALSE_IF_FAILED(result, "Failed to map staging memory for copying");
        memcpy(stagingData, desc.data, static_cast<size_t>(desc.dataSize));
        vkUnmapMemory(gDevice->GetDevice(), stagingMemory);

        result = vkBindBufferMemory(gDevice->GetDevice(), staging, stagingMemory, 0);
        RETURN_FALSE_IF_FAILED(result, "Failed to bind staging memory to staging buffer");

        // copy data from staging buffer to device
        // TODO OPTIMIZE this uses graphics queue and waits; after implementing queue manager, switch to Transfer queue
        CommandBuffer copyCmdBuffer;
        if (!copyCmdBuffer.Init())
            return false;

        copyCmdBuffer.Begin();
        copyCmdBuffer.CopyBuffer(staging, mBuffer, deviceMemReqs.size);
        copyCmdBuffer.End();

        gDevice->Execute(&copyCmdBuffer);
        gDevice->WaitForGPU(); // TODO this should be removed

        // cleanup
        vkFreeMemory(gDevice->GetDevice(), stagingMemory, nullptr);
        vkDestroyBuffer(gDevice->GetDevice(), staging, nullptr);
    }

    const char* memTypeStr = nullptr;
    switch (desc.type)
    {
    case BufferType::Static:
        memTypeStr = "Static";
        break;
    case BufferType::Dynamic:
        memTypeStr = "Dynamic";
        break;
    case BufferType::Volatile:
        memTypeStr = "Volatile";
        break;
    }

    mType = desc.type;
    LOGI(std::dec << mBufferSize << "-byte " << memTypeStr << " Buffer initialized successfully");
    return true;
}

bool Buffer::Write(const void* data, size_t size)
{
    if (mType == BufferType::Static)
    {
        LOGE("Cannot write to static buffer.");
        return false;
    }

    void* memory;
    VkResult result = vkMapMemory(gDevice->GetDevice(), mBufferMemory, 0, mBufferSize, 0, &memory);
    RETURN_FALSE_IF_FAILED(result, "Failed to map memory for writing");
    memcpy(memory, data, size);
    vkUnmapMemory(gDevice->GetDevice(), mBufferMemory);

    return true;
}

} // namespace Renderer
} // namespace ABench
