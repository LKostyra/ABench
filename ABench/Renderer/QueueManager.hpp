#pragma once

#include "Common/Common.hpp"


namespace ABench {
namespace Renderer {

enum DeviceQueueType
{
    GRAPHICS = 0,
    TRANSFER,
    COMPUTE,
    COUNT // used only as a counter for std::array definition below
};

// The manager for queues possible to acquire from VkPhysicalDevice.
// Gives Device class access to available queues and determines whether
// hardware supports Asynchronous Compute.
class QueueManager
{
public:
    using QueueCreateInfos = std::vector<VkDeviceQueueCreateInfo>;

private:
    static const uint32_t INVALID_QUEUE_INDEX;

    struct DeviceQueue
    {
        uint32_t index;
        VkQueue queue;
        VkCommandPool commandPool;

        DeviceQueue()
            : index(INVALID_QUEUE_INDEX)
            , queue(VK_NULL_HANDLE)
            , commandPool(VK_NULL_HANDLE)
        {
        }
    };

    using QueueFamilyProperties = std::vector<VkQueueFamilyProperties>;
    using DeviceQueues = std::array<DeviceQueue, DeviceQueueType::COUNT>;

    VkDevice mDevice;
    QueueFamilyProperties mQueueProperties;
    QueueCreateInfos mQueueCreateInfos;
    DeviceQueues mQueues;
    bool mSeparateTransferQueue;
    bool mSeparateComputeQueue;

    uint32_t GetQueueIndex(VkQueueFlags requestedFlag);

public:
    QueueManager();
    ~QueueManager();

    bool Init(VkPhysicalDevice physicalDevice);
    bool CreateQueues(VkDevice device);
    void Release();

    ABENCH_INLINE const QueueCreateInfos& GetQueueCreateInfos() const
    {
        return mQueueCreateInfos;
    }

    ABENCH_INLINE VkQueue GetQueue(DeviceQueueType queueType) const
    {
        return mQueues[queueType].queue;
    }

    ABENCH_INLINE VkCommandPool GetCommandPool(DeviceQueueType queueType) const
    {
        return mQueues[queueType].commandPool;
    }
};

} // namespace ABench
} // namespace Renderer
