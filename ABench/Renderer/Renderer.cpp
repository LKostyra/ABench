#include "../PCH.hpp"
#include "Renderer.hpp"

#include "Common/Logger.hpp"


namespace ABench {
namespace Renderer {

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
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

    ABench::Renderer::BackbufferDesc bbDesc;
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

    // TODO move to Tools
    ABench::Renderer::RenderPassDesc rpDesc;
    rpDesc.devicePtr = &mDevice;
    rpDesc.colorFormat = bbDesc.requestedFormat;
    if (!mRenderPass.Init(rpDesc))
        return false;

    // TODO move to Tools
    ABench::Renderer::FramebufferDesc fbDesc;
    fbDesc.devicePtr = &mDevice;
    fbDesc.colorTex = &mBackbuffer;
    fbDesc.renderPass = &mRenderPass;
    if (!mFramebuffer.Init(fbDesc))
        return false;

    float vertices[] =
    {
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // 1
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // 2
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, // 3

        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // 1
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, // 3
        -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // 4
    };

    ABench::Renderer::BufferDesc vbDesc;
    vbDesc.devicePtr = &mDevice;
    vbDesc.data = vertices;
    vbDesc.dataSize = sizeof(vertices);
    vbDesc.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (!mVertexBuffer.Init(vbDesc))
        return false;

    ABench::Renderer::VertexLayoutDesc vlDesc;

    std::vector<ABench::Renderer::VertexLayoutEntry> vlEntries;
    vlEntries.push_back({VK_FORMAT_R32G32B32_SFLOAT, 0, 0, 28, false});
    vlEntries.push_back({VK_FORMAT_R32G32B32A32_SFLOAT, 12, 0, 28, false});

    vlDesc.entryCount = static_cast<uint32_t>(vlEntries.size());
    vlDesc.entries = vlEntries.data();
    if (!mVertexLayout.Init(vlDesc))
        return false;

    ABench::Renderer::ShaderDesc shaderDesc;
    shaderDesc.devicePtr = &mDevice;
    shaderDesc.language = ABench::Renderer::ShaderLanguage::SPIRV;
    shaderDesc.path = "Data/Shaders/vert.spv";
    if (!mVertexShader.Init(shaderDesc))
        return false;

    shaderDesc.path = "Data/Shaders/frag.spv";
    if (!mFragmentShader.Init(shaderDesc))
        return false;

    mPipelineLayout = mTools.CreatePipelineLayout(nullptr, 0);
    if (mPipelineLayout == VK_NULL_HANDLE)
        return false;

    ABench::Renderer::PipelineDesc pipeDesc;
    pipeDesc.devicePtr = &mDevice;
    pipeDesc.vertexShader = &mVertexShader;
    pipeDesc.fragmentShader = &mFragmentShader;
    pipeDesc.vertexLayout = &mVertexLayout;
    pipeDesc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipeDesc.renderPass = &mRenderPass;
    pipeDesc.pipelineLayout = mPipelineLayout;
    mPipeline.Init(pipeDesc);

    if (!mCommandBuffer.Init(&mDevice))
        return false;

    return true;
}

void Renderer::Draw()
{
    {
        mCommandBuffer.Begin();

        mCommandBuffer.SetViewport(0, 0, mBackbuffer.GetWidth(), mBackbuffer.GetHeight(), 0.0f, 1.0f);
        mCommandBuffer.SetScissor(0, 0, mBackbuffer.GetWidth(), mBackbuffer.GetHeight());

        float clearValue[] = {0.2f, 0.4f, 0.8f, 0.0f};
        mCommandBuffer.BeginRenderPass(&mRenderPass, &mFramebuffer, clearValue);

        mCommandBuffer.BindPipeline(&mPipeline);
        mCommandBuffer.BindVertexBuffer(&mVertexBuffer);
        mCommandBuffer.Draw(6);

        mCommandBuffer.EndRenderPass();

        if (!mCommandBuffer.End())
        {
            LOGE("Failure during Command Buffer recording");
            return;
        }
    }


    mDevice.Execute(&mCommandBuffer);

    mBackbuffer.Present();

    mDevice.WaitForGPU();
}

} // namespace Renderer
} // namespace ABench
