#pragma once

#include "Common/Common.hpp"


namespace ABench {
namespace Renderer {

// The manager for queues possible to acquire from VkPhysicalDevice.
// Gives Device class access to available queues and determines whether
// hardware supports Asynchronous Compute.
class QueueManager
{
    using QueueFamilyProperties = std::vector<VkQueueFamilyProperties>;
    using QueueCreateInfos = std::vector<VkDeviceQueueCreateInfo>;

    QueueFamilyProperties mQueueProperties;
    QueueCreateInfos mQueueCreateInfos;
    uint32_t mGraphicsQueueIndex;
    uint32_t mComputeQueueIndex;

    uint32_t GetQueueIndex(VkQueueFlags requestedFlag);

public:
    QueueManager();
    ~QueueManager();

    bool Init(VkPhysicalDevice physicalDevice);
    void Release();

    ABENCH_INLINE uint32_t GetGraphicsQueueIndex() const
    {
        return mGraphicsQueueIndex;
    }

    ABENCH_INLINE uint32_t GetComputeQueueIndex() const
    {
        return mComputeQueueIndex;
    }

    ABENCH_INLINE const QueueCreateInfos& GetQueueCreateInfos() const
    {
        return mQueueCreateInfos;
    }
};

} // namespace ABench
} // namespace Renderer
