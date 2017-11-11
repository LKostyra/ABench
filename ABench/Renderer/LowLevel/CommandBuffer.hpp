#pragma once

#include "Prerequisites.hpp"
#include "Device.hpp"
#include "Framebuffer.hpp"
#include "Pipeline.hpp"
#include "Buffer.hpp"
#include "Device.hpp"
#include "Types.hpp"


namespace ABench {
namespace Renderer {

class CommandBuffer
{
    friend class Device;
    friend class Texture;

    DevicePtr mDevice;
    VkCommandPool mOwningPool;
    VkCommandBuffer mCommandBuffer;
    Framebuffer* mCurrentFramebuffer;

public:
    CommandBuffer();
    ~CommandBuffer();

    bool Init(const DevicePtr& device, DeviceQueueType queueType);

    void Begin();
    void BeginRenderPass(VkRenderPass rp, Framebuffer* fb, ClearType types, float clearValues[4], float depthValue = 0.0f);
    void BindVertexBuffer(const Buffer* buffer);
    void BindIndexBuffer(const Buffer* buffer);
    void BindPipeline(VkPipeline pipeline, VkPipelineBindPoint point);
    void BindDescriptorSet(VkDescriptorSet set, uint32_t setSlot, VkPipelineLayout layout);
    void BindDescriptorSet(VkDescriptorSet set, uint32_t setSlot, VkPipelineLayout layout, uint32_t dynamicOffset);
    void Clear(ClearType types, float clearValues[4], float depthValue);
    void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
    void CopyBufferToTexture(Buffer* src, Texture* dst);
    void Dispatch(uint32_t x, uint32_t y, uint32_t z);
    void Draw(uint32_t vertCount);
    void DrawIndexed(uint32_t vertCount);
    void EndRenderPass();
    bool End();
    void SetViewport(int32_t x, int32_t y, uint32_t width, uint32_t height, float minDepth, float maxDepth);
    void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);
};

} // namespace Renderer
} // namespace ABench
