#pragma once

#include "Device.hpp"
#include "Texture.hpp"
#include "RenderPass.hpp"


namespace ABench {
namespace Renderer {

struct FramebufferDesc
{
    Texture* colorTex;
    Texture* depthTex;
    RenderPass* renderPass;

    FramebufferDesc()
        : colorTex(nullptr)
        , depthTex(nullptr)
        , renderPass(nullptr)
    {
    }
};

class Framebuffer
{
    const Device* mDevicePtr;

    std::vector<VkFramebuffer> mFramebuffers;
    uint32_t mWidth;
    uint32_t mHeight;

public:
    Framebuffer(const Device* device);
    ~Framebuffer();

    bool Init(const FramebufferDesc& desc);
};

} // namespace Renderer
} // namespace ABench
