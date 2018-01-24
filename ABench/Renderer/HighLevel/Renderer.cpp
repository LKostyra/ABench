#include "PCH.hpp"
#include "Renderer.hpp"

#include "Renderer/LowLevel/DescriptorAllocator.hpp"
#include "Renderer/LowLevel/Extensions.hpp"
#include "Renderer/LowLevel/Translations.hpp"

#include "Common/Logger.hpp"
#include "Math/Plane.hpp"

#include "DescriptorLayoutManager.hpp"

#include <glslang/Public/ShaderLang.h>


namespace {

struct VertexShaderCBuffer
{
    ABench::Math::Matrix viewMatrix;
    ABench::Math::Matrix projMatrix;
};

struct VertexShaderDynamicCBuffer
{
    ABench::Math::Matrix worldMatrix;
};

} // namespace


namespace ABench {
namespace Renderer {


Renderer::Renderer()
    : mInstance(nullptr)
    , mDevice(nullptr)
    , mBackbuffer()
    , mImageAcquiredSem()
    , mRenderFinishedSem()
    , mFrameFence()
    , mVertexShaderSet(VK_NULL_HANDLE)
    , mVertexShaderCBuffer()
    , mGridFrustumsGenerator()
    , mDepthPrePass()
    , mForwardPass()
{
}

Renderer::~Renderer()
{
    glslang::FinalizeProcess();
}

bool Renderer::Init(const RendererDesc& desc)
{
    if (!glslang::InitializeProcess())
    {
        LOGE("Failed to initialize glslang");
        return false;
    }

    VkDebugReportFlagsEXT debugFlags = 0;

    if (desc.debugEnable)
    {
        debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT;
        if (desc.debugVerbose)
            debugFlags = VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;
    }

    mInstance = std::make_shared<Instance>();
    if (!mInstance->Init(debugFlags))
        return false;

    mDevice = std::make_shared<Device>();
    if (!mDevice->Init(mInstance))
        return false;

    // initialize Descriptor Allocator
    DescriptorAllocatorDesc daDesc;
    ZERO_MEMORY(daDesc);
    daDesc.limits[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] = 1;
    daDesc.limits[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] = 3;
    daDesc.limits[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] = 3;
    daDesc.limits[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] = 1000;
    if (!DescriptorAllocator::Instance().Init(mDevice, daDesc))
        return false;

    if (!ResourceManager::Instance().Init(mDevice))
        return false;

    if (!mGridFrustumsGenerator.Init(mDevice))
        return false;

    if (!DescriptorLayoutManager::Instance().Init(mDevice))
        return false;

    BackbufferWindowDesc bbWindowDesc;
#ifdef WIN32
    bbWindowDesc.hInstance = desc.window->GetInstance();
    bbWindowDesc.hWnd = desc.window->GetHandle();
#elif defined(__linux__) | defined(__LINUX__)
    bbWindowDesc.connection = desc.window->GetConnection();
    bbWindowDesc.window = desc.window->GetWindow();
#else
#error "Target platform not supported"
#endif

    BackbufferDesc bbDesc;
    bbDesc.instancePtr = mInstance;
    bbDesc.windowDesc = bbWindowDesc;
    bbDesc.requestedFormat = VK_FORMAT_B8G8R8A8_UNORM;
    bbDesc.width = desc.window->GetWidth();
    bbDesc.height = desc.window->GetHeight();
    bbDesc.vsync = false;
    if (!mBackbuffer.Init(mDevice, bbDesc))
        return false;

    // Synchronization primitives
    mImageAcquiredSem = Tools::CreateSem(mDevice);
    if (!mImageAcquiredSem)
        return false;

    mDepthFinishedSem = Tools::CreateSem(mDevice);
    if (!mDepthFinishedSem)
        return false;

    mRenderFinishedSem = Tools::CreateSem(mDevice);
    if (!mRenderFinishedSem)
        return false;

    mDepthFence = Tools::CreateFence(mDevice, true);
    if (!mDepthFence)
        return false;

    mFrameFence = Tools::CreateFence(mDevice, true);
    if (!mFrameFence)
        return false;

    // View frustum definition
    float aspect = static_cast<float>(desc.window->GetWidth()) / static_cast<float>(desc.window->GetHeight());
    mProjection = Math::CreateRHProjectionMatrix(desc.fov, aspect, desc.nearZ, desc.farZ);
    Math::FrustumDesc fdesc;
    fdesc.fov = desc.fov;
    fdesc.ratio = aspect;
    fdesc.nearZ = desc.nearZ;
    fdesc.farZ = desc.farZ;
    mViewFrustum.Init(fdesc);

    // Common shader descriptor sets & buffers
    if (!mRingBuffer.Init(mDevice, 1024 * 1024))
        return false;

    mVertexShaderSet = DescriptorAllocator::Instance().AllocateDescriptorSet(DescriptorLayoutManager::Instance().GetVertexShaderLayout());
    if (mVertexShaderSet == VK_NULL_HANDLE)
        return false;

    BufferDesc vsBufferDesc;
    vsBufferDesc.data = nullptr;
    vsBufferDesc.dataSize = sizeof(VertexShaderCBuffer);
    vsBufferDesc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vsBufferDesc.type = BufferType::Dynamic;
    if (!mVertexShaderCBuffer.Init(mDevice, vsBufferDesc))
        return false;


    // Point vertex shader set bindings to our dynamic buffer
    Tools::UpdateBufferDescriptorSet(mDevice, mVertexShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0,
                                     mRingBuffer.GetVkBuffer(), sizeof(VertexShaderDynamicCBuffer));
    Tools::UpdateBufferDescriptorSet(mDevice, mVertexShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                                     mVertexShaderCBuffer.GetBuffer(), sizeof(VertexShaderCBuffer));

    // Rendering passes
    GridFrustumsGenerationDesc genDesc;
    genDesc.projMat = mProjection;
    genDesc.viewportWidth = desc.window->GetWidth();
    genDesc.viewportHeight = desc.window->GetHeight();
    if (!mGridFrustumsGenerator.Generate(genDesc))
        return false;

    DepthPrePassDesc dppDesc;
    dppDesc.width = mBackbuffer.GetWidth();
    dppDesc.height = mBackbuffer.GetHeight();
    if (!mDepthPrePass.Init(mDevice, dppDesc))
        return false;

    ForwardPassDesc fpDesc;
    fpDesc.width = mBackbuffer.GetWidth();
    fpDesc.height = mBackbuffer.GetHeight();
    fpDesc.outputFormat = mBackbuffer.GetFormat();
    fpDesc.depthTexture = mDepthPrePass.GetDepthTexture();
    fpDesc.ringBufferPtr = &mRingBuffer;
    if (!mForwardPass.Init(mDevice, fpDesc))
        return false;

    return true;
}

void Renderer::Draw(const Scene::Scene& scene, const Scene::Camera& camera)
{
    // Perform view frustum culling for next scene
    mViewFrustum.Refresh(camera.GetPosition(), camera.GetAtPosition(), camera.GetUpVector());
    scene.ForEachObject([&](const Scene::Object* o) -> bool {
        if (o->GetComponent()->GetType() == Scene::ComponentType::Model)
        {
            Scene::Model* model = dynamic_cast<Scene::Model*>(o->GetComponent());
            o->SetToRender(mViewFrustum.Intersects(o->GetTransform() * model->GetAABB()));
        }

        return true;
    });

    // Wait for previous frame
    VkFence fences[] = { mFrameFence };
    VkResult result = vkWaitForFences(mDevice->GetDevice(), 1, fences, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS)
        LOGW("Failed to wait for fence: " << result << " (" << TranslateVkResultToString(result) << ")");

    result = vkResetFences(mDevice->GetDevice(), 1, fences);
    if (result != VK_SUCCESS)
        LOGW("Failed to reset frame fence: " << result << " (" << TranslateVkResultToString(result) << ")");

    // Update common descriptor set
    VertexShaderCBuffer vsBuffer;
    vsBuffer.viewMatrix = camera.GetView();
    vsBuffer.projMatrix = mProjection;
    if (!mVertexShaderCBuffer.Write(&vsBuffer, sizeof(vsBuffer)))
        LOGW("Failed to update Vertex Shader Uniform Buffer");

    // Rendering
    DepthPrePassDrawDesc depthDesc;
    depthDesc.ringBufferPtr = &mRingBuffer;
    depthDesc.vertexShaderSet = mVertexShaderSet;
    depthDesc.signalSem = mDepthFinishedSem;
    mDepthPrePass.Draw(scene, camera, depthDesc);

    if (!mBackbuffer.AcquireNextImage(mImageAcquiredSem))
        LOGE("Failed to acquire next image for rendering");

    ForwardPassDrawDesc forwardDesc;
    forwardDesc.ringBufferPtr = &mRingBuffer;
    forwardDesc.vertexShaderSet = mVertexShaderSet;
    forwardDesc.waitFlags = { VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    forwardDesc.waitSems = { mDepthFinishedSem, mImageAcquiredSem };
    forwardDesc.signalSem = mRenderFinishedSem;
    forwardDesc.fence = mFrameFence;
    mForwardPass.Draw(scene, camera, forwardDesc);

    mRingBuffer.MarkFinishedFrame();

    if (!mBackbuffer.Present(mForwardPass.GetTargetTexture(), mRenderFinishedSem))
        LOGE("Error during image presentation");
}

void Renderer::WaitForAll() const
{
    mDevice->Wait(DeviceQueueType::GRAPHICS);
    mDevice->Wait(DeviceQueueType::COMPUTE);
    mDevice->Wait(DeviceQueueType::TRANSFER);
}

} // namespace Renderer
} // namespace ABench
