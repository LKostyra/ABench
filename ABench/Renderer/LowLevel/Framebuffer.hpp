#pragma once

#include "Prerequisites.hpp"
#include "Device.hpp"
#include "Texture.hpp"
#include "Common/Common.hpp"


namespace ABench {
namespace Renderer {

struct FramebufferDesc
{
    Texture* colorTex;
    Texture* depthTex;
    VkRenderPass renderPass;

    FramebufferDesc()
        : colorTex(nullptr)
        , depthTex(nullptr)
        , renderPass(VK_NULL_HANDLE)
    {
    }
};

class Framebuffer
{
    friend class CommandBuffer;

    DevicePtr mDevice;

    VkFramebuffer mFramebuffer;
    uint32_t mWidth;
    uint32_t mHeight;
    Texture* mTexturePtr;
    Texture* mDepthTexturePtr;

public:
    Framebuffer();
    ~Framebuffer();

    bool Init(const DevicePtr& device, const FramebufferDesc& desc);

    ABENCH_INLINE uint32_t GetWidth() const
    {
        return mWidth;
    }

    ABENCH_INLINE uint32_t GetHeight() const
    {
        return mHeight;
    }
};

} // namespace Renderer
} // namespace ABench
