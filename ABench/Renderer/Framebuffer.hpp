#pragma once

#include "Prerequisites.hpp"
#include "Device.hpp"
#include "Texture.hpp"


namespace ABench {
namespace Renderer {

struct FramebufferDesc
{
    Device* devicePtr;

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

    Device* mDevicePtr;

    std::vector<VkFramebuffer> mFramebuffers;
    uint32_t mWidth;
    uint32_t mHeight;
    Texture* mTexturePtr;
    Texture* mDepthTexturePtr;

public:
    Framebuffer();
    ~Framebuffer();

    bool Init(const FramebufferDesc& desc);
};

} // namespace Renderer
} // namespace ABench
