#include "../PCH.hpp"
#include "Renderer.hpp"

#include "Extensions.hpp"
#include "Common/Logger.hpp"
#include "Math/Matrix.hpp"

namespace ABench {
namespace Renderer {


// TODO move to Camera class
struct VertexShaderCBuffer
{
    Math::Matrix viewMatrix;
    Math::Matrix projMatrix;
};


Renderer::Renderer()
    : mRenderPass(VK_NULL_HANDLE)
    , mPipelineLayout(VK_NULL_HANDLE)
    , mVertexShaderSet(VK_NULL_HANDLE)
    , mVertexShaderLayout(VK_NULL_HANDLE)
    , mDescriptorPool(VK_NULL_HANDLE)
{
}

Renderer::~Renderer()
{
    if (mVertexShaderLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(mDevice.GetDevice(), mVertexShaderLayout, nullptr);
    if (mDescriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(mDevice.GetDevice(), mDescriptorPool, nullptr);
    if (mRenderPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(mDevice.GetDevice(), mRenderPass, nullptr);
    if (mPipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(mDevice.GetDevice(), mPipelineLayout, nullptr);
}

bool Renderer::Init(const Common::Window& window, bool debugEnable, bool debugVerbose)
{
    VkDebugReportFlagsEXT debugFlags = 0;

    if (debugEnable)
    {
        debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT;
        if (debugVerbose)
            debugFlags = VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;
    }

    if (!mInstance.Init(debugFlags))
        return false;

    if (!mDevice.Init(mInstance))
        return false;

    if (!mTools.Init(&mDevice))
        return false;

    BackbufferDesc bbDesc;
    bbDesc.instancePtr = &mInstance;
    bbDesc.devicePtr = &mDevice;
    bbDesc.hInstance = window.GetInstance();
    bbDesc.hWnd = window.GetHandle();
    bbDesc.requestedFormat = VK_FORMAT_B8G8R8A8_UNORM;
    bbDesc.width = window.GetWidth();
    bbDesc.height = window.GetHeight();
    bbDesc.vsync = true;
    if (!mBackbuffer.Init(bbDesc))
        return false;

    mRenderPass = mTools.CreateRenderPass(bbDesc.requestedFormat, VK_FORMAT_UNDEFINED);
    if (mRenderPass == VK_NULL_HANDLE)
        return false;

    FramebufferDesc fbDesc;
    fbDesc.devicePtr = &mDevice;
    fbDesc.colorTex = &mBackbuffer;
    fbDesc.renderPass = mRenderPass;
    if (!mFramebuffer.Init(fbDesc))
        return false;

    float vertices[] =
    {
        -0.5f,-0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, // 0        7----6
         0.5f,-0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, // 1      3----2 |
         0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, // 2      | 4--|-5
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, // 3      0----1

        -0.5f,-0.5f,-0.5f, 1.0f, 0.0f, 0.0f, 1.0f, // 4
         0.5f,-0.5f,-0.5f, 0.0f, 1.0f, 0.0f, 1.0f, // 5
         0.5f, 0.5f,-0.5f, 1.0f, 1.0f, 0.0f, 1.0f, // 6
        -0.5f, 0.5f,-0.5f, 0.0f, 0.0f, 1.0f, 1.0f, // 7
    };

    uint32_t indices[] =
    {
        0, 1, 2, 0, 2, 3, // front
        3, 2, 6, 3, 6, 7, // top
        7, 6, 5, 7, 5, 4, // back
        4, 5, 1, 4, 1, 0, // bottom
        1, 5, 6, 1, 6, 2, // right
        4, 0, 3, 4, 3, 7, // left
    };

    BufferDesc vbDesc;
    vbDesc.devicePtr = &mDevice;
    vbDesc.data = vertices;
    vbDesc.dataSize = sizeof(vertices);
    vbDesc.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vbDesc.type = BufferType::Static;
    if (!mVertexBuffer.Init(vbDesc))
        return false;

    BufferDesc ibDesc;
    ibDesc.devicePtr = &mDevice;
    ibDesc.data = indices;
    ibDesc.dataSize = sizeof(indices);
    ibDesc.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    ibDesc.type = BufferType::Static;
    if (!mIndexBuffer.Init(ibDesc))
        return false;

    VertexLayoutDesc vlDesc;

    std::vector<VertexLayoutEntry> vlEntries;
    vlEntries.push_back({VK_FORMAT_R32G32B32_SFLOAT, 0, 0, 28, false});
    vlEntries.push_back({VK_FORMAT_R32G32B32A32_SFLOAT, 12, 0, 28, false});

    vlDesc.entryCount = static_cast<uint32_t>(vlEntries.size());
    vlDesc.entries = vlEntries.data();
    if (!mVertexLayout.Init(vlDesc))
        return false;

    ShaderDesc shaderDesc;
    shaderDesc.devicePtr = &mDevice;
    shaderDesc.language = ABench::Renderer::ShaderLanguage::SPIRV;
    shaderDesc.path = "Data/Shaders/vert.spv";
    if (!mVertexShader.Init(shaderDesc))
        return false;
    shaderDesc.path = "Data/Shaders/frag.spv";
    if (!mFragmentShader.Init(shaderDesc))
        return false;

    // shader resources initialization
    mVertexShaderLayout = mTools.CreateDescriptorSetLayout(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    if (mVertexShaderLayout == VK_NULL_HANDLE)
        return false;

    mPipelineLayout = mTools.CreatePipelineLayout(&mVertexShaderLayout, 1);
    if (mPipelineLayout == VK_NULL_HANDLE)
        return false;

    std::vector<VkDescriptorPoolSize> poolSizes;
    VkDescriptorPoolSize poolSize;

    // vertex shader binding - one uniform buffer set
    poolSize.descriptorCount = 1;
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes.push_back(poolSize);

    mDescriptorPool = mTools.CreateDescriptorPool(poolSizes);
    if (mDescriptorPool == VK_NULL_HANDLE)
        return false;

    mVertexShaderSet = mTools.AllocateDescriptorSet(mDescriptorPool, mVertexShaderLayout);
    if (mVertexShaderSet == VK_NULL_HANDLE)
        return false;

    PipelineDesc pipeDesc;
    pipeDesc.devicePtr = &mDevice;
    pipeDesc.vertexShader = &mVertexShader;
    pipeDesc.fragmentShader = &mFragmentShader;
    pipeDesc.vertexLayout = &mVertexLayout;
    pipeDesc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipeDesc.renderPass = mRenderPass;
    pipeDesc.pipelineLayout = mPipelineLayout;
    mPipeline.Init(pipeDesc);

    if (!mCommandBuffer.Init(&mDevice))
        return false;


    VertexShaderCBuffer cbuffer;
    cbuffer.viewMatrix = Math::CreateRotationMatrixZ(1.0f);

    BufferDesc vsBufferDesc;
    vsBufferDesc.devicePtr = &mDevice;
    vsBufferDesc.data = &cbuffer;
    vsBufferDesc.dataSize = sizeof(VertexShaderCBuffer);
    vsBufferDesc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vsBufferDesc.type = BufferType::Dynamic;
    if (!mVertexShaderCBuffer.Init(vsBufferDesc))
        return false;

    // Point vertex shader set to our dynamic buffer
    mTools.UpdateBufferDescriptorSet(mVertexShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, mVertexShaderCBuffer);

    return true;
}

void Renderer::Draw(const Scene::Camera& camera)
{
    // Update viewport
    VertexShaderCBuffer buf;
    buf.viewMatrix = camera.GetView();
    buf.projMatrix = camera.GetProjection();
    mVertexShaderCBuffer.Write(&buf, sizeof(VertexShaderCBuffer));


    // Rendering
    if (!mBackbuffer.AcquireNextImage())
        LOGE("Failed to acquire next image for rendering");

    {
        mCommandBuffer.Begin();

        mCommandBuffer.SetViewport(0, 0, mBackbuffer.GetWidth(), mBackbuffer.GetHeight(), 0.0f, 1.0f);
        mCommandBuffer.SetScissor(0, 0, mBackbuffer.GetWidth(), mBackbuffer.GetHeight());

        float clearValue[] = {0.2f, 0.4f, 0.8f, 0.0f};
        mCommandBuffer.BeginRenderPass(mRenderPass, &mFramebuffer, clearValue);

        mCommandBuffer.BindPipeline(&mPipeline);
        mCommandBuffer.BindVertexBuffer(&mVertexBuffer);
        mCommandBuffer.BindIndexBuffer(&mIndexBuffer);
        mCommandBuffer.BindDescriptorSet(mVertexShaderSet, mPipelineLayout);
        mCommandBuffer.DrawIndexed(36);
        mCommandBuffer.EndRenderPass();

        if (!mCommandBuffer.End())
        {
            LOGE("Failure during Command Buffer recording");
            return;
        }
    }

    mDevice.Execute(&mCommandBuffer);

    if (!mBackbuffer.Present())
        LOGE("Error during image presentation");

    mDevice.WaitForGPU();
}

} // namespace Renderer
} // namespace ABench
