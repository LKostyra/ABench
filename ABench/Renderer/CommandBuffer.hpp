#pragma once

#include "Device.hpp"
#include "RenderPass.hpp"
#include "Framebuffer.hpp"
#include "Pipeline.hpp"
#include "Buffer.hpp"


namespace ABench {
namespace Renderer {

class Buffer;

class CommandBuffer
{
    friend class Device;

    const Device* mDevicePtr;

    VkCommandBuffer mCommandBuffer;
    Framebuffer* mCurrentFramebuffer;

public:
    CommandBuffer(const Device* device);
    ~CommandBuffer();

    bool Init();

    void Begin();
    void BeginRenderPass(RenderPass* rp, Framebuffer* fb, float clearValues[4]);
    void BindVertexBuffer(Buffer* buffer);
    void BindPipeline(Pipeline* pipeline);
    void Clear(float clearValues[4]);
    void Draw(uint32_t vertCount);
    void EndRenderPass();
    bool End();
    void SetViewport(int32_t x, int32_t y, uint32_t width, uint32_t height, float minDepth, float maxDepth);
    void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);
};

} // namespace Renderer
} // namespace ABench
