#pragma once

#include "Renderer/LowLevel/CommandBuffer.hpp"
#include "Renderer/LowLevel/DescriptorAllocator.hpp"
#include "Renderer/LowLevel/MultiPipeline.hpp"
#include "Renderer/LowLevel/Tools.hpp"
#include "Renderer/LowLevel/Device.hpp"
#include "Renderer/LowLevel/Buffer.hpp"
#include "Renderer/LowLevel/Texture.hpp"

#include "Scene/Scene.hpp"


namespace ABench {
namespace Renderer {

struct LightCullerDesc
{
    uint32_t viewportWidth;
    uint32_t viewportHeight;
    uint32_t pixelsPerGridFrustum;
    Buffer* gridFrustums;
    Buffer* lightContainer;
    Texture* depthTexture;

    LightCullerDesc()
        : viewportWidth(0)
        , viewportHeight(0)
        , pixelsPerGridFrustum(0)
        , gridFrustums(nullptr)
        , lightContainer(nullptr)
        , depthTexture(nullptr)
    {
    }
};


struct LightCullerDispatchDesc
{
    uint32_t lightCount;
    std::vector<VkPipelineStageFlags> waitFlags;
    std::vector<VkSemaphore> waitSems;
    VkSemaphore signalSem;
};


class LightCuller final
{
    struct CullingParams
    {
        uint32_t viewportWidth;
        uint32_t viewportHeight;
        uint32_t frustumsPerWidth;
        uint32_t frustumsPerHeight;
        uint32_t lightCount;

        CullingParams()
            : viewportWidth(0)
            , viewportHeight(0)
            , frustumsPerWidth(0)
            , frustumsPerHeight(0)
            , lightCount(0)
        {
        }
    };

    DevicePtr mDevice;

    Buffer mCullingParams;
    Buffer mCulledLights;
    Buffer mGridLightData;

    VkDescriptorSet mLightCullerSet;
    VkRAII<VkDescriptorSetLayout> mDescriptorSetLayout;
    VkRAII<VkPipelineLayout> mPipelineLayout;
    MultiPipeline mPipeline;
    CommandBuffer mCommandBuffer;

    CullingParams mCullingParamsData;
    uint32_t mPixelsPerGridFrustum;

public:
    LightCuller();

    bool Init(const DevicePtr& device, const LightCullerDesc& desc);
    void Dispatch(const LightCullerDispatchDesc& desc);

    ABENCH_INLINE Buffer* GetCulledLights()
    {
        return &mCulledLights;
    }

    ABENCH_INLINE Buffer* GetGridLightData()
    {
        return &mGridLightData;
    }
};

} // namespace Renderer
} // namespace ABench
