#include "PCH.hpp"
#include "Renderer.hpp"

#include "Extensions.hpp"
#include "Tools.hpp"
#include "DescriptorLayoutManager.hpp"
#include "ShaderMacroDefinitions.hpp"

#include "Common/Logger.hpp"
#include "Math/Matrix.hpp"

#include <glslang/Public/ShaderLang.h>

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

struct FragmentShaderLightCBuffer
{
    Math::Vector pos;
    Math::Vector diffuse;
};

Device* gDevice = nullptr;


Renderer::Renderer()
    : mRenderPass(VK_NULL_HANDLE)
    , mPipelineLayout(VK_NULL_HANDLE)
    , mRenderFence(VK_NULL_HANDLE)
    , mVertexShaderSet(VK_NULL_HANDLE)
    , mFragmentShaderSet(VK_NULL_HANDLE)
{
}

Renderer::~Renderer()
{
    DescriptorLayoutManager::Instance().Release();

    if (mRenderFence != VK_NULL_HANDLE)
        vkDestroyFence(mDevice.GetDevice(), mRenderFence, nullptr);
    if (mRenderPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(mDevice.GetDevice(), mRenderPass, nullptr);
    if (mPipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(mDevice.GetDevice(), mPipelineLayout, nullptr);

    glslang::FinalizeProcess();
}

bool Renderer::Init(const Common::Window& window, bool debugEnable, bool debugVerbose)
{
    if (!glslang::InitializeProcess())
    {
        LOGE("Failed to initialize glslang");
        return false;
    }

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

    mRenderPass = Tools::CreateRenderPass(bbDesc.requestedFormat, VK_FORMAT_D32_SFLOAT);
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

    if (!DescriptorLayoutManager::Instance().Init(mDevice.GetDevice()))
        return false;

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.push_back(DescriptorLayoutManager::Instance().GetVertexShaderLayout());
    layouts.push_back(DescriptorLayoutManager::Instance().GetFragmentShaderDiffuseTextureLayout());
    layouts.push_back(DescriptorLayoutManager::Instance().GetFragmentShaderNormalTextureLayout());
    layouts.push_back(DescriptorLayoutManager::Instance().GetFragmentShaderLayout());
    mPipelineLayout = Tools::CreatePipelineLayout(layouts.data(), static_cast<uint32_t>(layouts.size()));
    if (mPipelineLayout == VK_NULL_HANDLE)
        return false;

    // buffer-related set allocation
    mVertexShaderSet = mDevice.GetDescriptorAllocator().AllocateDescriptorSet(DescriptorLayoutManager::Instance().GetVertexShaderLayout());
    if (mVertexShaderSet == VK_NULL_HANDLE)
        return false;

    mFragmentShaderSet = mDevice.GetDescriptorAllocator().AllocateDescriptorSet(DescriptorLayoutManager::Instance().GetFragmentShaderLayout());
    if (mFragmentShaderSet == VK_NULL_HANDLE)
        return false;


    PipelineDesc pipeDesc;
    pipeDesc.vertexLayout = &mVertexLayout;
    pipeDesc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipeDesc.renderPass = mRenderPass;
    pipeDesc.pipelineLayout = mPipelineLayout;
    pipeDesc.enableDepth = true;

    MultiPipelineDesc mpDesc;
    mpDesc.vertexShader.path = "shader.vert";
    mpDesc.fragmentShader.path = "shader.frag";
    mpDesc.fragmentShader.macros = {
        { ShaderMacro::HAS_TEXTURE, 1 },
        { ShaderMacro::HAS_NORMAL, 1 },
    };
    mpDesc.pipelineDesc = pipeDesc;

    if (!mPipeline.Init(mpDesc))
        return false;


    if (!mCommandBuffer.Init())
        return false;


    BufferDesc vsBufferDesc;
    vsBufferDesc.data = nullptr;
    vsBufferDesc.dataSize = sizeof(VertexShaderCBuffer);
    vsBufferDesc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vsBufferDesc.type = BufferType::Dynamic;
    if (!mVertexShaderCBuffer.Init(vsBufferDesc))
        return false;

    BufferDesc fsBufferDesc;
    fsBufferDesc.data = nullptr;
    fsBufferDesc.dataSize = sizeof(FragmentShaderLightCBuffer);
    fsBufferDesc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    fsBufferDesc.type = BufferType::Dynamic;
    if (!mFragmentShaderLightCBuffer.Init(vsBufferDesc))
        return false;

    // Point vertex shader set bindings to our dynamic buffer
    Tools::UpdateBufferDescriptorSet(mVertexShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0,
                                     mRingBuffer.GetVkBuffer(), sizeof(VertexShaderDynamicCBuffer));
    Tools::UpdateBufferDescriptorSet(mVertexShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                                     mVertexShaderCBuffer.GetVkBuffer(), sizeof(VertexShaderCBuffer));
    Tools::UpdateBufferDescriptorSet(mFragmentShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
                                     mFragmentShaderLightCBuffer.GetVkBuffer(), sizeof(FragmentShaderLightCBuffer));


    mRenderFence = Tools::CreateFence();
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

    FragmentShaderLightCBuffer lightBuf;

    scene.ForEachLight([&lightBuf](const Scene::Object* o) -> bool {
        // gather only first light's position for now
        Scene::Light* l = dynamic_cast<Scene::Light*>(o->GetComponent());

        lightBuf.pos = o->GetPosition();
        lightBuf.diffuse = l->GetDiffuseIntensity();
        return false;
    });
    mFragmentShaderLightCBuffer.Write(&lightBuf, sizeof(FragmentShaderLightCBuffer));


    // Rendering
    if (!mBackbuffer.AcquireNextImage())
        LOGE("Failed to acquire next image for rendering");

    {
        mCommandBuffer.Begin();

        mCommandBuffer.SetViewport(0, 0, mBackbuffer.GetWidth(), mBackbuffer.GetHeight(), 0.0f, 1.0f);
        mCommandBuffer.SetScissor(0, 0, mBackbuffer.GetWidth(), mBackbuffer.GetHeight());

        float clearValue[] = {0.1f, 0.1f, 0.1f, 0.0f};
        mCommandBuffer.BeginRenderPass(mRenderPass, &mFramebuffer,
                                       static_cast<ClearTypes>(ABENCH_CLEAR_COLOR | ABENCH_CLEAR_DEPTH), clearValue, 1.0f);
        mCommandBuffer.BindDescriptorSet(mFragmentShaderSet, 3, mPipelineLayout);

        MultiPipelineShaderMacros macros;
        macros.fragmentShader = {
            { ShaderMacro::HAS_TEXTURE, 0 },
            { ShaderMacro::HAS_NORMAL, 0 }
        };

        scene.ForEachObject([&](const Scene::Object* o) -> bool {
            if (o->GetComponent()->GetType() == Scene::ComponentType::Model)
            {
                uint32_t offset = mRingBuffer.Write(&o->GetTransform(), sizeof(ABench::Math::Matrix));
                mCommandBuffer.BindDescriptorSet(mVertexShaderSet, 0, mPipelineLayout, offset);

                Scene::Model* model = dynamic_cast<Scene::Model*>(o->GetComponent());
                model->ForEachMesh([&](Scene::Mesh* mesh) {
                    if (mesh->GetMaterial() != nullptr)
                    {
                        macros.fragmentShader[0].value = 1;
                        macros.fragmentShader[1].value = 0;

                        if (mesh->GetMaterial()->GetDiffuseDescriptor() != VK_NULL_HANDLE)
                        {
                            mCommandBuffer.BindDescriptorSet(mesh->GetMaterial()->GetDiffuseDescriptor(), 1, mPipelineLayout);
                        }

                        if (mesh->GetMaterial()->GetNormalDescriptor() != VK_NULL_HANDLE)
                        {
                            macros.fragmentShader[1].value = 1;
                            mCommandBuffer.BindDescriptorSet(mesh->GetMaterial()->GetNormalDescriptor(), 2, mPipelineLayout);
                        }
                    }
                    else
                        macros.fragmentShader[0].value = 0;

                    mCommandBuffer.BindPipeline(mPipeline.GetPipelineWithShaders(macros));
                    mCommandBuffer.BindVertexBuffer(mesh->GetVertexBuffer());

                    if (mesh->ByIndices())
                    {
                        mCommandBuffer.BindIndexBuffer(mesh->GetIndexBuffer());
                        mCommandBuffer.DrawIndexed(mesh->GetPointCount());
                    }
                    else
                    {
                        mCommandBuffer.Draw(mesh->GetPointCount());
                    }
                });
            }

            return true;
        });

        mCommandBuffer.EndRenderPass();

        if (!mCommandBuffer.End())
        {
            LOGE("Failure during Command Buffer recording");
            return;
        }
    }

    mDevice.Execute(&mCommandBuffer, mRenderFence);

    // TODO waiting is performed inside Ring Buffer on mRenderFence
    //      Escape from this limitation in the future
    mRingBuffer.MarkFinishedFrame(mRenderFence);

    if (!mBackbuffer.Present())
        LOGE("Error during image presentation");
}

} // namespace Renderer
} // namespace ABench
