#pragma once

#include "Common\Common.hpp"

namespace ABench {
namespace Renderer {

struct DescriptorSetLayoutDesc
{
    VkDescriptorType type;
    VkShaderStageFlags stage;
    VkSampler sampler;

    DescriptorSetLayoutDesc()
        : type(VK_DESCRIPTOR_TYPE_SAMPLER)
        , stage(0)
        , sampler(VK_NULL_HANDLE)
    {
    }

    DescriptorSetLayoutDesc(VkDescriptorType type, VkShaderStageFlags stage, VkSampler sampler)
        : type(type)
        , stage(stage)
        , sampler(sampler)
    {
    }
};

class DescriptorLayoutManager
{
    VkDevice mDevice;

    VkSampler mFragmentShaderSampler;

    VkDescriptorSetLayout mVertexShaderLayout;
    VkDescriptorSetLayout mFragmentShaderTextureLayout;
    VkDescriptorSetLayout mFragmentShaderLayout;

    DescriptorLayoutManager();
    DescriptorLayoutManager(const DescriptorLayoutManager&) = delete;
    DescriptorLayoutManager(DescriptorLayoutManager&&) = delete;
    DescriptorLayoutManager& operator=(const DescriptorLayoutManager&) = delete;
    DescriptorLayoutManager& operator=(DescriptorLayoutManager&&) = delete;
    ~DescriptorLayoutManager();

    VkDescriptorSetLayout CreateLayout(std::vector<DescriptorSetLayoutDesc>& descriptors);
    VkSampler CreateSampler();

public:
    static DescriptorLayoutManager& Instance();

    bool Init(const VkDevice device);
    void Release();

    ABENCH_INLINE const VkDescriptorSetLayout GetVertexShaderLayout() const
    {
        return mVertexShaderLayout;
    }

    ABENCH_INLINE const VkDescriptorSetLayout GetFragmentShaderTextureLayout() const
    {
        return mFragmentShaderTextureLayout;
    }

    ABENCH_INLINE const VkDescriptorSetLayout GetFragmentShaderLayout() const
    {
        return mFragmentShaderLayout;
    }
};

} // namespace Renderer
} // namespace ABench