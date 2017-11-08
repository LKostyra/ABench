#pragma once

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

class DescriptorLayoutManager
{
    VkDevice mDevice;

    VkSampler mFragmentShaderSampler;

    VkDescriptorSetLayout mVertexShaderLayout;
    VkDescriptorSetLayout mFragmentShaderLayout;
    VkDescriptorSetLayout mFragmentShaderDiffuseTextureLayout;
    VkDescriptorSetLayout mFragmentShaderNormalTextureLayout;
    VkDescriptorSetLayout mFragmentShaderMaskTextureLayout;
    VkDescriptorSetLayout mAllShaderLayout;

    DescriptorLayoutManager();
    DescriptorLayoutManager(const DescriptorLayoutManager&) = delete;
    DescriptorLayoutManager(DescriptorLayoutManager&&) = delete;
    DescriptorLayoutManager& operator=(const DescriptorLayoutManager&) = delete;
    DescriptorLayoutManager& operator=(DescriptorLayoutManager&&) = delete;
    ~DescriptorLayoutManager();

    void DestroyLayout(VkDescriptorSetLayout& layout);
    VkSampler CreateSampler();

public:
    static DescriptorLayoutManager& Instance();

    bool Init(const VkDevice device);
    void Release();

    ABENCH_INLINE const VkDescriptorSetLayout GetVertexShaderLayout() const
    {
        return mVertexShaderLayout;
    }

    ABENCH_INLINE const VkDescriptorSetLayout GetFragmentShaderLayout() const
    {
        return mFragmentShaderLayout;
    }

    ABENCH_INLINE const VkDescriptorSetLayout GetFragmentShaderDiffuseTextureLayout() const
    {
        return mFragmentShaderDiffuseTextureLayout;
    }

    ABENCH_INLINE const VkDescriptorSetLayout GetFragmentShaderNormalTextureLayout() const
    {
        return mFragmentShaderNormalTextureLayout;
    }

    ABENCH_INLINE const VkDescriptorSetLayout GetFragmentShaderMaskTextureLayout() const
    {
        return mFragmentShaderMaskTextureLayout;
    }

    ABENCH_INLINE const VkDescriptorSetLayout GetAllShaderLayout() const
    {
        return mAllShaderLayout;
    }
};

} // namespace Renderer
} // namespace ABench
