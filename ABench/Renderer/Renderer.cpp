#include "PCH.hpp"
#include "Renderer.hpp"

#include "Extensions.hpp"
#include "Common/Logger.hpp"
#include "Math/Matrix.hpp"

namespace ABench {
namespace Renderer {

struct VertexShaderDynamicCBuffer
{
    Math::Matrix worldMatrix;
};

// TODO move to Camera class
struct VertexShaderCBuffer
{
    Math::Matrix viewMatrix;
    Math::Matrix projMatrix;
};

Device* gDevice = nullptr;


Renderer::Renderer()
    : mRenderPass(VK_NULL_HANDLE)
    , mPipelineLayout(VK_NULL_HANDLE)
    , mRenderFence(VK_NULL_HANDLE)
    , mVertexShaderSet(VK_NULL_HANDLE)
    , mVertexShaderLayout(VK_NULL_HANDLE)
    , mDescriptorPool(VK_NULL_HANDLE)
{
}

Renderer::~Renderer()
{
    if (mRenderFence != VK_NULL_HANDLE)
        vkDestroyFence(mDevice.GetDevice(), mRenderFence, nullptr);
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

    // TODO hack to avoid providing Device for all objects
    // consider maybe a better solution (Device singleton?)
    gDevice = &mDevice;

    BackbufferWindowDesc bbWindowDesc;
#ifdef WIN32
    bbWindowDesc.hInstance = window.GetInstance();
    bbWindowDesc.hWnd = window.GetHandle();
#elif defined(__linux__) | defined(__LINUX__)
    bbWindowDesc.connection = window.GetConnection();
    bbWindowDesc.window = window.GetWindow();
#else
#error "Target platform not supported"
#endif

    BackbufferDesc bbDesc;
    bbDesc.instancePtr = &mInstance;
    bbDesc.windowDesc = bbWindowDesc;
    bbDesc.requestedFormat = VK_FORMAT_B8G8R8A8_UNORM;
    bbDesc.width = window.GetWidth();
    bbDesc.height = window.GetHeight();
    bbDesc.vsync = false;
    if (!mBackbuffer.Init(bbDesc))
        return false;

    TextureDesc depthTexDesc;
    depthTexDesc.width = window.GetWidth();
    depthTexDesc.height = window.GetHeight();
    depthTexDesc.format = VK_FORMAT_D32_SFLOAT;
    depthTexDesc.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (!mDepthTexture.Init(depthTexDesc))
        return false;

    if (!mRingBuffer.Init(1024*1024))
        return false;

    mRenderPass = mTools.CreateRenderPass(bbDesc.requestedFormat, VK_FORMAT_D32_SFLOAT);
    if (mRenderPass == VK_NULL_HANDLE)
        return false;

    FramebufferDesc fbDesc;
    fbDesc.colorTex = &mBackbuffer;
    fbDesc.depthTex = &mDepthTexture;
    fbDesc.renderPass = mRenderPass;
    if (!mFramebuffer.Init(fbDesc))
        return false;

    VertexLayoutDesc vlDesc;

    std::vector<VertexLayoutEntry> vlEntries;
    vlEntries.push_back({VK_FORMAT_R32G32B32_SFLOAT, 0, 0, 32, false}); // vertex position
    vlEntries.push_back({VK_FORMAT_R32G32B32_SFLOAT, 12, 0, 32, false}); // vertex normal
    vlEntries.push_back({VK_FORMAT_R32G32_SFLOAT, 24, 0, 32, false}); // vertex uv

    vlDesc.entryCount = static_cast<uint32_t>(vlEntries.size());
    vlDesc.entries = vlEntries.data();
    if (!mVertexLayout.Init(vlDesc))
        return false;

    ShaderDesc shaderDesc;
    shaderDesc.language = ABench::Renderer::ShaderLanguage::SPIRV;
    shaderDesc.path = "Data/Shaders/shader.vert.spv";
    if (!mVertexShader.Init(shaderDesc))
        return false;
    shaderDesc.path = "Data/Shaders/shader.frag.spv";
    if (!mFragmentShader.Init(shaderDesc))
        return false;

    // shader resources initialization
    std::vector<DescriptorSetLayoutDesc> layoutDesc;
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, VK_NULL_HANDLE});
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_NULL_HANDLE});
    mVertexShaderLayout = mTools.CreateDescriptorSetLayout(layoutDesc);
    if (mVertexShaderLayout == VK_NULL_HANDLE)
        return false;

    mPipelineLayout = mTools.CreatePipelineLayout(&mVertexShaderLayout, 1);
    if (mPipelineLayout == VK_NULL_HANDLE)
        return false;

    // vertex shader binding - one uniform buffer set with two bindings
    std::vector<VkDescriptorPoolSize> poolSizes;

    VkDescriptorPoolSize poolSize;
    poolSize.descriptorCount = 1;
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes.push_back(poolSize);
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes.push_back(poolSize);

    mDescriptorPool = mTools.CreateDescriptorPool(poolSizes);
    if (mDescriptorPool == VK_NULL_HANDLE)
        return false;

    // set allocation
    mVertexShaderSet = mTools.AllocateDescriptorSet(mDescriptorPool, mVertexShaderLayout);
    if (mVertexShaderSet == VK_NULL_HANDLE)
        return false;

    PipelineDesc pipeDesc;
    pipeDesc.vertexShader = &mVertexShader;
    pipeDesc.fragmentShader = &mFragmentShader;
    pipeDesc.vertexLayout = &mVertexLayout;
    pipeDesc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipeDesc.renderPass = mRenderPass;
    pipeDesc.pipelineLayout = mPipelineLayout;
    pipeDesc.enableDepth = true;
    mPipeline.Init(pipeDesc);

    if (!mCommandBuffer.Init())
        return false;


    VertexShaderCBuffer cbuffer;
    cbuffer.viewMatrix = Math::CreateRotationMatrixZ(1.0f);

    BufferDesc vsBufferDesc;
    vsBufferDesc.data = &cbuffer;
    vsBufferDesc.dataSize = sizeof(VertexShaderCBuffer);
    vsBufferDesc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vsBufferDesc.type = BufferType::Dynamic;
    if (!mVertexShaderCBuffer.Init(vsBufferDesc))
        return false;

    // Point vertex shader set to our dynamic buffer
    mTools.UpdateBufferDescriptorSet(mVertexShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0,
                                     mRingBuffer.GetVkBuffer(), sizeof(VertexShaderDynamicCBuffer));
    mTools.UpdateBufferDescriptorSet(mVertexShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                                     mVertexShaderCBuffer.GetVkBuffer(), sizeof(VertexShaderCBuffer));

    mRenderFence = mTools.CreateFence();
    if (mRenderFence == VK_NULL_HANDLE)
        return false;

    return true;
}

void Renderer::Draw(const Scene::Scene& scene, const Scene::Camera& camera)
{
    // Update viewport
    // TODO view could be pushed to dynamic buffer for optimization
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
        mCommandBuffer.BeginRenderPass(mRenderPass, &mFramebuffer,
                                       static_cast<ClearTypes>(ABENCH_CLEAR_COLOR | ABENCH_CLEAR_DEPTH), clearValue, 1.0f);
        mCommandBuffer.BindPipeline(&mPipeline);

        scene.ForEachObject([&](const Scene::Object* o) {
            if (o->GetComponent()->GetType() == Scene::ComponentType::Mesh)
            {
                uint32_t offset = mRingBuffer.Write(&o->GetTransform(), sizeof(ABench::Math::Matrix));
                mCommandBuffer.BindDescriptorSet(mVertexShaderSet, mPipelineLayout, offset);

                Scene::Mesh* mesh = dynamic_cast<Scene::Mesh*>(o->GetComponent());
                mCommandBuffer.BindVertexBuffer(mesh->GetVertexBuffer());
                mCommandBuffer.BindIndexBuffer(mesh->GetIndexBuffer());
                mCommandBuffer.DrawIndexed(mesh->GetIndexCount());
            }
        });

        mCommandBuffer.EndRenderPass();

        if (!mCommandBuffer.End())
        {
            LOGE("Failure during Command Buffer recording");
            return;
        }
    }

    mDevice.Execute(&mCommandBuffer, mRenderFence);

    if (!mBackbuffer.Present())
        LOGE("Error during image presentation");

    // TODO waiting is performed inside Ring Buffer on mRenderFence
    //      Escape from this limitation in the future
    mRingBuffer.MarkFinishedFrame(mRenderFence);
}

} // namespace Renderer
} // namespace ABench
