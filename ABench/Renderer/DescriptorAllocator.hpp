#pragma once

namespace ABench {
namespace Renderer {

struct DescriptorAllocatorDesc
{
    uint32_t limits[VK_DESCRIPTOR_TYPE_RANGE_SIZE];

    DescriptorAllocatorDesc()
    {
        for (uint32_t i = 0; i < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++i)
            limits[i] = 0;
    }
};

struct DescriptorPool
{
    VkDescriptorPool pool;
    uint32_t limits[VK_DESCRIPTOR_TYPE_RANGE_SIZE];
    uint32_t taken[VK_DESCRIPTOR_TYPE_RANGE_SIZE];

    DescriptorPool()
        : pool(VK_NULL_HANDLE)
    {
        for (uint32_t i = 0; i < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++i)
        {
            limits[i] = 0;
            taken[i] = 0;
        }
    }
};

// A wrapper for VkDescriptorPool objects.
// Holds preallocated Descriptor Pools with high-limit descriptor sets.
class DescriptorAllocator
{
    VkDevice mDevice;

    uint32_t mMaxSets;
    DescriptorAllocatorDesc mLimits;
    std::vector<VkDescriptorPoolSize> mPoolSizes;

    std::vector<DescriptorPool> mDescriptorPools;

    bool AllocateNewPool();

public:
    DescriptorAllocator();
    ~DescriptorAllocator();

    bool Init(VkDevice device, const DescriptorAllocatorDesc& desc);
    void Release();

    VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetLayout layout);
};

} // namespace Renderer
} // namespace ABench
