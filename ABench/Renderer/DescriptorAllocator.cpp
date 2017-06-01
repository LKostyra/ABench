#include "PCH.hpp"
#include "DescriptorAllocator.hpp"

#include "Util.hpp"
#include "Extensions.hpp"
#include "Renderer.hpp"

#include "Common/Common.hpp"
#include "Common/Logger.hpp"


namespace ABench {
namespace Renderer {

DescriptorAllocator::DescriptorAllocator()
    : mMaxSets(0)
{
}

DescriptorAllocator::~DescriptorAllocator()
{
    Release();
}

bool DescriptorAllocator::AllocateNewPool()
{
    mDescriptorPools.emplace_back();

    VkDescriptorPoolCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.poolSizeCount = static_cast<uint32_t>(mPoolSizes.size());
    info.pPoolSizes = mPoolSizes.data();
    info.maxSets = mMaxSets;
    VkResult result = vkCreateDescriptorPool(mDevice, &info, nullptr, &mDescriptorPools.back().pool);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Descriptor Pool");

    LOGD("Created Descriptor Pool 0x" << std::hex << reinterpret_cast<size_t*>(mDescriptorPools.back().pool));

    for (uint32_t i = 0; i < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++i)
    {
        mDescriptorPools.back().limits[i] = mLimits.limits[i];
        mDescriptorPools.back().taken[i] = 0;
    }

    return true;
}

bool DescriptorAllocator::Init(VkDevice device, const DescriptorAllocatorDesc& desc)
{
    mDevice = device;

    VkDescriptorPoolSize size;
    for (uint32_t i = 0; i < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++i)
    {
        mLimits.limits[i] = desc.limits[i];
        mMaxSets += mLimits.limits[i];
        if (mLimits.limits[i] > 0)
        {
            size.descriptorCount = mLimits.limits[i];
            size.type = static_cast<VkDescriptorType>(i);
            mPoolSizes.push_back(size);
        }
    }

    // TODO check if we do not exceed device's limits
    return AllocateNewPool();
}

void DescriptorAllocator::Release()
{
    for (auto& p: mDescriptorPools)
        vkDestroyDescriptorPool(mDevice, p.pool, nullptr);

    mDescriptorPools.clear();
}

VkDescriptorSet DescriptorAllocator::AllocateDescriptorSet(VkDescriptorSetLayout layout)
{
    // TODO memory tracking (somehow)

    VkDescriptorSet set = VK_NULL_HANDLE;

    VkDescriptorSetAllocateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorPool = mDescriptorPools.back().pool;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &layout;
    VkResult result = vkAllocateDescriptorSets(mDevice, &info, &set);
    // TODO add new pool if limits are exceeded
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to allocate Descriptor Set");

    return set;
}

} // namespace Renderer
} // namespace ABench
