#pragma once

#include "Device.hpp"
#include "Texture.hpp"


namespace ABench {
namespace Renderer {

class Texture;

struct FramebufferDesc
{
    Device* devicePtr;

    Texture* colorTex;
    Texture* depthTex;
    VkRenderPass renderPass;

    FramebufferDesc()
        : colorTex(nullptr)
        , depthTex(nullptr)
        , renderPass(nullptr)
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

public:
    Framebuffer();
    ~Framebuffer();

    bool Init(const FramebufferDesc& desc);
};

} // namespace Renderer
} // namespace ABench
