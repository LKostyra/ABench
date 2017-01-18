#include "../PCH.hpp"
#include "Buffer.hpp"
#include "Util.hpp"
#include "Extensions.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

Buffer::Buffer(const Device* device)
    : mDevicePtr(device)
{
}

Buffer::~Buffer()
{
    if (mBufferMemory)
        vkFreeMemory(mDevicePtr->mDevice, mBufferMemory, nullptr);
    if (mBuffer)
        vkDestroyBuffer(mDevicePtr->mDevice, mBuffer, nullptr);
}

bool Buffer::Init(const BufferDesc& desc)
{
    if (!desc.data || desc.dataSize == 0)
    {
        LOGE("Invalid/empty data provided for Buffer initialization");
        return false;
    }


    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    VkBuffer staging = VK_NULL_HANDLE;

    mBufferSize = desc.dataSize;

    // create a staging buffer
    VkBufferCreateInfo bufInfo;
    ZERO_MEMORY(bufInfo);
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = mBufferSize;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkResult result = vkCreateBuffer(mDevicePtr->mDevice, &bufInfo, nullptr, &staging);
    CHECK_VKRESULT(result, "Failed to create staging buffer");

    // get staging buffer's memory requirements
    VkMemoryRequirements stagingMemReqs;
    vkGetBufferMemoryRequirements(mDevicePtr->mDevice, staging, &stagingMemReqs);

    // allocate memory for staging buffer
    VkMemoryAllocateInfo memInfo;
    ZERO_MEMORY(memInfo);
    memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memInfo.allocationSize = stagingMemReqs.size;
    memInfo.memoryTypeIndex = mDevicePtr->GetMemoryTypeIndex(stagingMemReqs.memoryTypeBits,
                                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    result = vkAllocateMemory(mDevicePtr->mDevice, &memInfo, nullptr, &stagingMemory);
    CHECK_VKRESULT(result, "Failed to allocate memory for staging buffer");

    // map the memory and copy data to it
    void* stagingData = nullptr;
    result = vkMapMemory(mDevicePtr->mDevice, stagingMemory, 0, stagingMemReqs.size, 0, &stagingData);
    CHECK_VKRESULT(result, "Failed to map staging memory for copying");
    memcpy(stagingData, desc.data, static_cast<size_t>(desc.dataSize));
    vkUnmapMemory(mDevicePtr->mDevice, stagingMemory);

    result = vkBindBufferMemory(mDevicePtr->mDevice, staging, stagingMemory, 0);
    CHECK_VKRESULT(result, "Failed to bind staging memory to staging buffer");


    // repeat above steps for device buffer (except map-copy-unmap)
    bufInfo.usage = desc.usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    result = vkCreateBuffer(mDevicePtr->mDevice, &bufInfo, nullptr, &mBuffer);
    CHECK_VKRESULT(result, "Failed to create device buffer");

    VkMemoryRequirements deviceMemReqs;
    vkGetBufferMemoryRequirements(mDevicePtr->mDevice, mBuffer, &deviceMemReqs);

    memInfo.memoryTypeIndex = mDevicePtr->GetMemoryTypeIndex(deviceMemReqs.memoryTypeBits,
                                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    result = vkAllocateMemory(mDevicePtr->mDevice, &memInfo, nullptr, &mBufferMemory);
    CHECK_VKRESULT(result, "Failed to allocate device memory");

    result = vkBindBufferMemory(mDevicePtr->mDevice, mBuffer, mBufferMemory, 0);
    CHECK_VKRESULT(result, "Failed to bind device memory to device buffer");


    // copy data from staging buffer to device
    // TODO OPTIMIZE this uses graphics queue and waits; after implementing queue manager, switch to Transfer queue
    VkCommandBuffer copyCommand;
    VkCommandBufferAllocateInfo cmdInfo;
    ZERO_MEMORY(cmdInfo);
    cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdInfo.commandPool = mDevicePtr->mCommandPool;
    cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdInfo.commandBufferCount = 1;
    result = vkAllocateCommandBuffers(mDevicePtr->mDevice, &cmdInfo, &copyCommand);
    CHECK_VKRESULT(result, "Failed to allcoate command buffer for copy between staging and device");

    VkCommandBufferBeginInfo cmdBeginInfo;
    ZERO_MEMORY(cmdBeginInfo);
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    result = vkBeginCommandBuffer(copyCommand, &cmdBeginInfo);
    CHECK_VKRESULT(result, "Failed to begin copy command buffer");

    VkBufferCopy region;
    ZERO_MEMORY(region);
    region.size = deviceMemReqs.size;
    vkCmdCopyBuffer(copyCommand, staging, mBuffer, 1, &region);

    result = vkEndCommandBuffer(copyCommand);
    CHECK_VKRESULT(result, "Error during copy command buffer record");

    VkSubmitInfo submitInfo;
    ZERO_MEMORY(submitInfo);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &copyCommand;
    vkQueueSubmit(mDevicePtr->mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

    mDevicePtr->WaitForGPU();


    // cleanup
    vkFreeCommandBuffers(mDevicePtr->mDevice, mDevicePtr->mCommandPool, 1, &copyCommand);
    vkFreeMemory(mDevicePtr->mDevice, stagingMemory, nullptr);
    vkDestroyBuffer(mDevicePtr->mDevice, staging, nullptr);

    LOGI(std::dec << mBufferSize << "-byte Buffer initialized successfully");
    return true;
}

} // namespace Renderer
} // namespace ABench
