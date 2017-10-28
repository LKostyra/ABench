#pragma once

#include "Instance.hpp"
#include "Device.hpp"
#include "Backbuffer.hpp"
#include "Texture.hpp"
#include "Framebuffer.hpp"
#include "Buffer.hpp"
#include "Shader.hpp"
#include "CommandBuffer.hpp"
#include "VertexLayout.hpp"
#include "RingBuffer.hpp"
#include "MultiPipeline.hpp"

#include "Common/Window.hpp"

#include "Scene/Camera.hpp"
#include "Scene/Mesh.hpp"
#include "Scene/Scene.hpp"


namespace ABench {
namespace Renderer {

class Renderer final
{
    Instance mInstance;
    Device mDevice;
    Backbuffer mBackbuffer;
    Texture mDepthTexture;
    Framebuffer mFramebuffer;
    VertexLayout mVertexLayout;
    MultiPipeline mPipeline;
    CommandBuffer mCommandBuffer;
    RingBuffer mRingBuffer;

    VkRenderPass mRenderPass;
    VkPipelineLayout mPipelineLayout;
    VkFence mRenderFence;

    VkDescriptorSet mVertexShaderSet;
    VkDescriptorSet mFragmentShaderSet;
    VkDescriptorSet mAllShaderSet;
    Buffer mVertexShaderCBuffer;
    Buffer mAllShaderLightCBuffer;

public:
    Renderer();
    ~Renderer();

    bool Init(const Common::Window& window, bool debugEnable = false, bool debugVerbose = false);
    void Draw(const Scene::Scene& scene, const Scene::Camera& camera);
};

// TODO hack to enable global device for all objects
// consider something else
extern Device* gDevice;

} // namespace Renderer
} // namespace ABench
