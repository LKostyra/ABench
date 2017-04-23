#pragma once

#include "Device.hpp"
#include "Framebuffer.hpp"
#include "Pipeline.hpp"
#include "Buffer.hpp"


namespace ABench {
namespace Renderer {

class Buffer;
class Pipeline;
class Framebuffer;

class CommandBuffer
{
    friend class Device;

    Device* mDevicePtr;

    VkCommandBuffer mCommandBuffer;
    Framebuffer* mCurrentFramebuffer;

public:
    CommandBuffer();
    ~CommandBuffer();

    bool Init(Device* devicePtr);

    void Begin();
    void BeginRenderPass(VkRenderPass rp, Framebuffer* fb, float clearValues[4]);
    void BindVertexBuffer(const Buffer* buffer);
    void BindIndexBuffer(const Buffer* buffer);
    void BindPipeline(Pipeline* pipeline);
    void BindDescriptorSet(VkDescriptorSet set, VkPipelineLayout layout);
    void BindDescriptorSet(VkDescriptorSet set, VkPipelineLayout layout, uint32_t dynamicOffset);
    void Clear(float clearValues[4]);
    void Draw(uint32_t vertCount);
    void DrawIndexed(uint32_t vertCount);
    void EndRenderPass();
    bool End();
    void SetViewport(int32_t x, int32_t y, uint32_t width, uint32_t height, float minDepth, float maxDepth);
    void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);
};

} // namespace Renderer
} // namespace ABench
