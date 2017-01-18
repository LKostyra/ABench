#pragma once

#include "Device.hpp"
#include "RenderPass.hpp"
#include "Framebuffer.hpp"


namespace ABench {
namespace Renderer {

class CommandBuffer
{
    friend class Device;

    const Device* mDevicePtr;

    VkCommandBuffer mCommandBuffer;

public:
    CommandBuffer(const Device* device);
    ~CommandBuffer();

    bool Init();

    void Begin();
    void BeginRenderPass(RenderPass* rp, Framebuffer* fb, float clearValues[4]);
    void EndRenderPass();
    bool End();
};

} // namespace Renderer
} // namespace ABench
