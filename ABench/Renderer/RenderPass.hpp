#pragma once

#include "Device.hpp"


namespace ABench {
namespace Renderer {

struct RenderPassDesc
{
    // TODO multiple render pass targets
    VkFormat colorFormat;
    VkFormat depthFormat;

    RenderPassDesc()
        : colorFormat(VK_FORMAT_UNDEFINED)
        , depthFormat(VK_FORMAT_UNDEFINED)
    {
    }
};

class RenderPass
{
    friend class Framebuffer;

    const Device* mDevicePtr;

    VkRenderPass mRenderPass;

public:
    RenderPass(const Device* device);
    ~RenderPass();

    bool Init(const RenderPassDesc& desc);
};

} // namespace Renderer
} // namespace ABench
