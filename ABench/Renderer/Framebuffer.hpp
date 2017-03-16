#pragma once

#include "Device.hpp"
#include "Texture.hpp"
#include "RenderPass.hpp"


namespace ABench {
namespace Renderer {

struct FramebufferDesc
{
    Device* devicePtr;

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
    friend class CommandBuffer;

    Device* mDevicePtr;

    std::vector<VkFramebuffer> mFramebuffers;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t* mCurrentBufferPtr;

public:
    Framebuffer();
    ~Framebuffer();

    bool Init(const FramebufferDesc& desc);
};

} // namespace Renderer
} // namespace ABench
