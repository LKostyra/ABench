#pragma once

#include "Instance.hpp"
#include "Device.hpp"
#include "Tools.hpp"
#include "Backbuffer.hpp"
#include "Texture.hpp"
#include "Framebuffer.hpp"
#include "Buffer.hpp"
#include "Shader.hpp"
#include "CommandBuffer.hpp"
#include "VertexLayout.hpp"
#include "RingBuffer.hpp"

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
    Tools mTools;
    Backbuffer mBackbuffer;
    Texture mDepthTexture;
    Framebuffer mFramebuffer;
    VertexLayout mVertexLayout;
    Shader mVertexShader;
    Shader mFragmentShader;
    Pipeline mPipeline;
    CommandBuffer mCommandBuffer;
    RingBuffer mRingBuffer;

    VkRenderPass mRenderPass;
    VkPipelineLayout mPipelineLayout;
    VkFence mRenderFence;

    VkDescriptorSet mVertexShaderSet;
    VkDescriptorSetLayout mVertexShaderLayout;
    VkDescriptorPool mDescriptorPool;
    Buffer mVertexShaderCBuffer;

public:
    Renderer();
    ~Renderer();

    bool Init(const Common::Window& window, bool debugEnable = false, bool debugVerbose = false);
    void Draw(const Scene::Scene& scene, const Scene::Camera& camera);

    ABENCH_INLINE Device* GetDevice()
    {
        return &mDevice;
    }
};

} // namespace Renderer
} // namespace ABench
