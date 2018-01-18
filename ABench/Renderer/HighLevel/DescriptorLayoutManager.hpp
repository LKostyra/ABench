#pragma once

#include "Common/Common.hpp"
#include "Renderer/LowLevel/Device.hpp"
#include "Renderer/LowLevel/Tools.hpp"

namespace ABench {
namespace Renderer {

class DescriptorLayoutManager
{
    DevicePtr mDevice;

    VkRAII<VkSampler> mFragmentShaderSampler;
    VkRAII<VkDescriptorSetLayout> mFragmentShaderDiffuseTextureLayout;
    VkRAII<VkDescriptorSetLayout> mFragmentShaderNormalTextureLayout;
    VkRAII<VkDescriptorSetLayout> mFragmentShaderMaskTextureLayout;

    DescriptorLayoutManager();
    DescriptorLayoutManager(const DescriptorLayoutManager&) = delete;
    DescriptorLayoutManager(DescriptorLayoutManager&&) = delete;
    DescriptorLayoutManager& operator=(const DescriptorLayoutManager&) = delete;
    DescriptorLayoutManager& operator=(DescriptorLayoutManager&&) = delete;
    ~DescriptorLayoutManager();

public:
    static DescriptorLayoutManager& Instance();

    bool Init(const DevicePtr& device);

    ABENCH_INLINE VkRAII<VkDescriptorSetLayout>& GetFragmentShaderDiffuseTextureLayout()
    {
        return mFragmentShaderDiffuseTextureLayout;
    }

    ABENCH_INLINE VkRAII<VkDescriptorSetLayout>& GetFragmentShaderNormalTextureLayout()
    {
        return mFragmentShaderNormalTextureLayout;
    }

    ABENCH_INLINE VkRAII<VkDescriptorSetLayout>& GetFragmentShaderMaskTextureLayout()
    {
        return mFragmentShaderMaskTextureLayout;
    }

    ABENCH_INLINE VkRAII<VkSampler>& GetSampler()
    {
        return mFragmentShaderSampler;
    }
};

} // namespace Renderer
} // namespace ABench
