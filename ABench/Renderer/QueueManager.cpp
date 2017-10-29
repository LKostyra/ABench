#include "PCH.hpp"
#include "QueueManager.hpp"

#include "Translations.hpp"
#include "Extensions.hpp"
#include "Common/Logger.hpp"



namespace {

const uint32_t INVALID_QUEUE_INDEX = 0xFFFFFFFF;

} // namespace


namespace ABench {
namespace Renderer {

QueueManager::QueueManager()
    : mQueueProperties()
    , mQueueCreateInfos()
    , mGraphicsQueueIndex(INVALID_QUEUE_INDEX)
    , mComputeQueueIndex(INVALID_QUEUE_INDEX)
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

    mGraphicsQueueIndex = GetQueueIndex(VK_QUEUE_GRAPHICS_BIT);
    mComputeQueueIndex = GetQueueIndex(VK_QUEUE_COMPUTE_BIT);

    if (mGraphicsQueueIndex == INVALID_QUEUE_INDEX || mComputeQueueIndex == INVALID_QUEUE_INDEX)
    {
        LOGE("Failed to acquire Graphics and Compute queue family indices");
        return false;
    }

    // gather queue creation information for further use
    float queuePriorities[] = { 1.0f };
    VkDeviceQueueCreateInfo queueInfo;
    ZERO_MEMORY(queueInfo);
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = queuePriorities;
    queueInfo.queueFamilyIndex = mGraphicsQueueIndex;
    mQueueCreateInfos.push_back(queueInfo);

    if (mComputeQueueIndex != mGraphicsQueueIndex)
    {
        queueInfo.queueFamilyIndex = mComputeQueueIndex;
        mQueueCreateInfos.push_back(queueInfo);
    }

    return true;
}

void QueueManager::Release()
{
    mQueueProperties.clear();
    mQueueCreateInfos.clear();
    mGraphicsQueueIndex = INVALID_QUEUE_INDEX;
    mComputeQueueIndex = INVALID_QUEUE_INDEX;
}

} // namespace ABench
} // namespace Renderer
