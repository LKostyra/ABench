#include "PCH.hpp"
#include "QueueManager.hpp"

#include "Translations.hpp"
#include "Extensions.hpp"
#include "Util.hpp"
#include "Common/Logger.hpp"



namespace ABench {
namespace Renderer {

const uint32_t QueueManager::INVALID_QUEUE_INDEX = 0xFFFFFFFF;

QueueManager::QueueManager()
    : mQueueProperties()
    , mQueueCreateInfos()
    , mQueues()
    , mQueuePriorities{1.0f}
    , mSeparateTransferQueue(false)
    , mSeparateComputeQueue(false)
{
}

QueueManager::~QueueManager()
{
    Release();
}

uint32_t QueueManager::GetQueueIndex(VkQueueFlags requestedFlag)
{
    uint32_t queueIndex = INVALID_QUEUE_INDEX;
    VkQueueFlags queueCapabilities = VK_QUEUE_FLAG_BITS_MAX_ENUM;

    // Acquired queue family index follows two rules:
    //   * Requested flag is a part of queue family's properties
    //   * It is the queue with lowest possible capabilities
    //
    // Main reasoning behind these rules is a fact that queue families in Vulkan ascend in
    // capabilities. For example, on old devices you have two queue families - one supports
    // every possible capability, the other supports only TRANSFER and SPARSE_BINDING operations.
    // If we would like to push TRANSFER ops independently from GRAPHICS ops, we have to use the less
    // capable queue family for TRANSFER operations, as GRAPHICS operations are pushed to the more
    // capable one. Otherwise, we would just enqueue all operations on one queue family, which would
    // ruin all multithreading capabilities of Vulkan.
    for (size_t i = 0; i < mQueueProperties.size(); ++i)
    {
        if ((mQueueProperties[i].queueFlags & requestedFlag) &&
            (mQueueProperties[i].queueFlags < queueCapabilities))
        {
            queueIndex = static_cast<uint32_t>(i);
            queueCapabilities = mQueueProperties[i].queueFlags;
        }
    }

    if (queueIndex == INVALID_QUEUE_INDEX)
    {
        LOGE("No queue found to match requested capabilities: " << TranslateVkQueueFlagsToString(requestedFlag));
    }

    return queueIndex;
}

bool QueueManager::Init(VkPhysicalDevice physicalDevice)
{
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);
    if (queueCount == 0)
    {
        LOGE("Physical device does not have any queue family properties.");
        return false;
    }

    mQueueProperties.resize(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, mQueueProperties.data());

    // debugging stuff
    for (uint32_t i = 0; i < queueCount; ++i)
    {
        LOGD("Queue Family #" << i << ":");
        LOGD("  Flags: " << std::hex << mQueueProperties[i].queueFlags << " ("
                         << TranslateVkQueueFlagsToString(mQueueProperties[i].queueFlags) << ")");
    }

    mQueues[DeviceQueueType::GRAPHICS].index = GetQueueIndex(VK_QUEUE_GRAPHICS_BIT);
    mQueues[DeviceQueueType::TRANSFER].index = GetQueueIndex(VK_QUEUE_TRANSFER_BIT);
    mQueues[DeviceQueueType::COMPUTE].index = GetQueueIndex(VK_QUEUE_COMPUTE_BIT);

    if (mQueues[DeviceQueueType::GRAPHICS].index == INVALID_QUEUE_INDEX ||
        mQueues[DeviceQueueType::TRANSFER].index == INVALID_QUEUE_INDEX ||
        mQueues[DeviceQueueType::COMPUTE].index == INVALID_QUEUE_INDEX)
    {
        LOGE("Failed to acquire queue family indices");
        return false;
    }

    mSeparateTransferQueue = (mQueues[DeviceQueueType::TRANSFER].index != mQueues[DeviceQueueType::GRAPHICS].index);
    mSeparateComputeQueue = (mQueues[DeviceQueueType::COMPUTE].index != mQueues[DeviceQueueType::GRAPHICS].index);

    // gather queue creation information for further use
    VkDeviceQueueCreateInfo queueInfo;
    ZERO_MEMORY(queueInfo);
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueCount = static_cast<uint32_t>(mQueuePriorities.size());
    queueInfo.pQueuePriorities = mQueuePriorities.data();
    queueInfo.queueFamilyIndex = mQueues[DeviceQueueType::GRAPHICS].index;
    mQueueCreateInfos.push_back(queueInfo);

    if (mSeparateTransferQueue)
    {
        queueInfo.queueFamilyIndex = mQueues[DeviceQueueType::TRANSFER].index;
        mQueueCreateInfos.push_back(queueInfo);
    }

    if (mSeparateComputeQueue)
    {
        queueInfo.queueFamilyIndex = mQueues[DeviceQueueType::COMPUTE].index;
        mQueueCreateInfos.push_back(queueInfo);
    }

    return true;
}

bool QueueManager::CreateQueues(VkDevice device)
{
    mDevice = device;

    // Acquire Queues from device
    vkGetDeviceQueue(mDevice, mQueues[DeviceQueueType::GRAPHICS].index, 0, &mQueues[DeviceQueueType::GRAPHICS].queue);
    vkGetDeviceQueue(mDevice, mQueues[DeviceQueueType::TRANSFER].index, 0, &mQueues[DeviceQueueType::TRANSFER].queue);
    vkGetDeviceQueue(mDevice, mQueues[DeviceQueueType::COMPUTE].index, 0, &mQueues[DeviceQueueType::COMPUTE].queue);

    // Create Command Pools
    VkCommandPoolCreateInfo poolInfo;
    ZERO_MEMORY(poolInfo);
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    // Graphics Queue (obligatory)
    poolInfo.queueFamilyIndex = mQueues[DeviceQueueType::GRAPHICS].index;
    VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &mQueues[DeviceQueueType::GRAPHICS].commandPool);
    RETURN_FALSE_IF_FAILED(result, "Failed to create Graphics Command Pool");

    // Transfer Queue (optional)
    if (mSeparateTransferQueue)
    {
        poolInfo.queueFamilyIndex = mQueues[DeviceQueueType::TRANSFER].index;
        result = vkCreateCommandPool(device, &poolInfo, nullptr, &mQueues[DeviceQueueType::TRANSFER].commandPool);
        RETURN_FALSE_IF_FAILED(result, "Failed to create Transfer Command Pool");
    }
    else
    {
        mQueues[DeviceQueueType::TRANSFER].commandPool = mQueues[DeviceQueueType::GRAPHICS].commandPool;
    }

    // Compute Queue (optional)
    if (mSeparateComputeQueue)
    {
        poolInfo.queueFamilyIndex = mQueues[DeviceQueueType::COMPUTE].index;
        result = vkCreateCommandPool(device, &poolInfo, nullptr, &mQueues[DeviceQueueType::COMPUTE].commandPool);
        RETURN_FALSE_IF_FAILED(result, "Failed to create Compute Command Pool");
    }
    else
    {
        mQueues[DeviceQueueType::COMPUTE].commandPool = mQueues[DeviceQueueType::GRAPHICS].commandPool;
    }

    return true;
}

void QueueManager::Release()
{
    mQueueProperties.clear();
    mQueueCreateInfos.clear();

    // first free the pools
    for (size_t i = 0; i < mQueues.size(); ++i)
    {
        if (mQueues[i].commandPool != VK_NULL_HANDLE)
            // only destroy a pool if it is of GRAPHICS type or if it is separate
            if ((i == DeviceQueueType::GRAPHICS) ||
                (mQueues[DeviceQueueType::GRAPHICS].index != mQueues[i].index))
                vkDestroyCommandPool(mDevice, mQueues[i].commandPool, nullptr);
    }

    for (auto& q: mQueues)
    {
        q.commandPool = VK_NULL_HANDLE;
        q.queue = VK_NULL_HANDLE;
        q.index = INVALID_QUEUE_INDEX;
    }
}

} // namespace ABench
} // namespace Renderer
