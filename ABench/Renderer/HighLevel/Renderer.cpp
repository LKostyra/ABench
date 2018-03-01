#include "PCH.hpp"
#include "Renderer.hpp"

#include "Renderer/LowLevel/DescriptorAllocator.hpp"
#include "Renderer/LowLevel/Extensions.hpp"
#include "Renderer/LowLevel/Translations.hpp"

#include "ResourceDir.hpp"
#include "Common/FS.hpp"
#include "Common/Logger.hpp"
#include "Math/Plane.hpp"

#include <glslang/Public/ShaderLang.h>


namespace {

const uint32_t PIXELS_PER_GRID_FRUSTUM = 16;

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
    , mParticleEngineSem()
    , mDepthSem()
    , mCullingSem()
    , mRenderSem()
    , mFrameFence()
    , mVertexShaderSet(VK_NULL_HANDLE)
    , mVertexShaderCBuffer()
    , mRingBuffer()
    , mLightContainer()
    , mGridFrustumsGenerator()
    , mDepthPrePass()
    , mLightCuller()
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
    if (!mDevice->Init(mInstance, desc.noAsync))
        return false;

    // initialize Descriptor Allocator
    DescriptorAllocatorDesc daDesc;
    ZERO_MEMORY(daDesc);
    daDesc.limits[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] = 12;
    daDesc.limits[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] = 7;
    daDesc.limits[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] = 3;
    daDesc.limits[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] = 1000;
    if (!DescriptorAllocator::Instance().Init(mDevice, daDesc))
        return false;

    if (!ResourceManager::Instance().Init(mDevice))
        return false;

    if (!mGridFrustumsGenerator.Init(mDevice))
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

    mParticleEngineSem = Tools::CreateSem(mDevice);
    if (!mParticleEngineSem)
        return false;

    mDepthSem = Tools::CreateSem(mDevice);
    if (!mDepthSem)
        return false;

    mCullingSem = Tools::CreateSem(mDevice);
    if (!mCullingSem)
        return false;

    mRenderSem = Tools::CreateSem(mDevice);
    if (!mRenderSem)
        return false;

    mParticlePassSem = Tools::CreateSem(mDevice);
    if (!mParticlePassSem)
        return false;

    mParticleEngineFence = Tools::CreateFence(mDevice, false);
    if (!mParticleEngineFence)
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

    // Common shader descriptor sets
    std::vector<DescriptorSetLayoutDesc> vsLayoutDesc;
    vsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, VK_NULL_HANDLE});
    vsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_NULL_HANDLE});
    mVertexShaderLayout = Tools::CreateDescriptorSetLayout(mDevice, vsLayoutDesc);
    if (!mVertexShaderLayout)
        return false;

    mVertexShaderSet = DescriptorAllocator::Instance().AllocateDescriptorSet(mVertexShaderLayout);
    if (mVertexShaderSet == VK_NULL_HANDLE)
        return false;

    // Common shader buffers
    if (!mRingBuffer.Init(mDevice, 1024 * 1024))
        return false;

    BufferDesc vsBufferDesc;
    vsBufferDesc.data = nullptr;
    vsBufferDesc.dataSize = sizeof(VertexShaderCBuffer);
    vsBufferDesc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vsBufferDesc.type = BufferType::Dynamic;
    if (!mVertexShaderCBuffer.Init(mDevice, vsBufferDesc))
        return false;

    BufferDesc lightBufferDesc;
    lightBufferDesc.data = nullptr;
    lightBufferDesc.dataSize = 1024 * 1024;
    lightBufferDesc.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    lightBufferDesc.type = BufferType::Dynamic;
    if (!mLightContainer.Init(mDevice, lightBufferDesc))
        return false;


    // Point vertex shader set bindings to our dynamic buffer
    Tools::UpdateBufferDescriptorSet(mDevice, mVertexShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0,
                                     mRingBuffer.GetBuffer(), sizeof(VertexShaderDynamicCBuffer));
    Tools::UpdateBufferDescriptorSet(mDevice, mVertexShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                                     mVertexShaderCBuffer.GetBuffer(), sizeof(VertexShaderCBuffer));


    // Rendering stages
    GridFrustumsGenerationDesc genDesc;
    genDesc.projMat = mProjection;
    genDesc.viewportWidth = desc.window->GetWidth();
    genDesc.viewportHeight = desc.window->GetHeight();
    genDesc.pixelsPerGridFrustum = PIXELS_PER_GRID_FRUSTUM;
    if (!mGridFrustumsGenerator.Generate(genDesc))
        return false;

    if (!mParticleEngine.Init(mDevice))
        return false;

    DepthPrePassDesc dppDesc;
    dppDesc.width = mBackbuffer.GetWidth();
    dppDesc.height = mBackbuffer.GetHeight();
    dppDesc.vertexShaderLayout = mVertexShaderLayout;
    if (!mDepthPrePass.Init(mDevice, dppDesc))
        return false;

    LightCullerDesc lcDesc;
    lcDesc.viewportWidth = desc.window->GetWidth();
    lcDesc.viewportHeight = desc.window->GetHeight();
    lcDesc.pixelsPerGridFrustum = PIXELS_PER_GRID_FRUSTUM;
    lcDesc.gridFrustums = mGridFrustumsGenerator.GetGridFrustums();
    lcDesc.lightContainer = &mLightContainer;
    lcDesc.depthTexture = mDepthPrePass.GetDepthTexture();
    if (!mLightCuller.Init(mDevice, lcDesc))
        return false;

    ForwardPassDesc fpDesc;
    fpDesc.width = mBackbuffer.GetWidth();
    fpDesc.height = mBackbuffer.GetHeight();
    fpDesc.outputFormat = mBackbuffer.GetFormat();
    fpDesc.vertexShaderLayout = mVertexShaderLayout;
    fpDesc.pixelsPerGridFrustum = PIXELS_PER_GRID_FRUSTUM;
    fpDesc.depthTexture = mDepthPrePass.GetDepthTexture();
    fpDesc.ringBufferPtr = &mRingBuffer;
    fpDesc.lightContainerPtr = &mLightContainer;
    fpDesc.culledLightsPtr = mLightCuller.GetCulledLights();
    fpDesc.gridLightDataPtr = mLightCuller.GetGridLightData();
    if (!mForwardPass.Init(mDevice, fpDesc))
        return false;

    ParticlePassDesc ppDesc;
    ppDesc.targetTexture = &mForwardPass.GetTargetTexture();
    ppDesc.depthTexture = mDepthPrePass.GetDepthTexture();
    ppDesc.vertexShaderBuffer = &mVertexShaderCBuffer;
    ppDesc.outputFormat = mBackbuffer.GetFormat();
    ppDesc.particleTexturePath = Common::FS::JoinPaths(
        Common::FS::JoinPaths(ResourceDir::DATA_ROOT, ResourceDir::TEXTURES), "particle.png");
    if (!mParticlePass.Init(mDevice, ppDesc))
        return false;

    return true;
}

void Renderer::Draw(const Scene::Scene& scene, const Scene::Camera& camera, float deltaTime)
{
    // Perform view frustum culling for next scene
    mViewFrustum.Refresh(camera.GetPosition(), camera.GetAtPosition(), camera.GetUpVector());
    scene.ForEachObject([&](const Scene::Object* o) -> bool {
        if (o->GetComponent()->GetType() == Scene::ComponentType::Model)
        {
            Scene::Model* model = dynamic_cast<Scene::Model*>(o->GetComponent());
            model->SetToRender(mViewFrustum.Intersects(model->GetTransform() * model->GetAABB()));
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


    ////////////////////////////
    // Particle Engine update //
    ////////////////////////////
    mParticleEngine.UpdateEmitters(scene);
    ParticleEngineDispatchDesc peDesc;
    peDesc.cameraPos = camera.GetPosition();
    peDesc.deltaTime = deltaTime;
    peDesc.signalSem = mParticleEngineSem;
    peDesc.simulationFence = mParticleEngineFence;
    mParticleEngine.Dispatch(peDesc);


    //////////////////////////////////
    // Rendering descriptors update //
    //////////////////////////////////
    VertexShaderCBuffer vsBuffer;
    vsBuffer.viewMatrix = camera.GetView();
    vsBuffer.projMatrix = mProjection;
    if (!mVertexShaderCBuffer.Write(&vsBuffer, sizeof(vsBuffer)))
        LOGW("Failed to update Vertex Shader Uniform Buffer");

    uint32_t lightCount = 0;
    scene.ForEachLight([&](const ABench::Scene::Light* l) -> bool {
        if (!mLightContainer.Write(l->GetData(), sizeof(Scene::LightData), lightCount * sizeof(Scene::LightData)))
            LOGW("Failed to update Light Container Storage Buffer");
        lightCount++;
        return true;
    });


    ///////////////
    // Rendering //
    ///////////////

    // Depth pass
    DepthPrePassDrawDesc depthDesc;
    depthDesc.ringBufferPtr = &mRingBuffer;
    depthDesc.vertexShaderSet = mVertexShaderSet;
    depthDesc.signalSem = mDepthSem;
    mDepthPrePass.Draw(scene, depthDesc);

    // Light culling dispatch
    LightCullerDispatchDesc cullingDesc;
    cullingDesc.lightCount = lightCount;
    cullingDesc.projMat = mProjection;
    cullingDesc.viewMat = camera.GetView();
    cullingDesc.waitFlags = { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };
    cullingDesc.waitSems = { mDepthSem };
    cullingDesc.signalSem = mCullingSem;
    mLightCuller.Dispatch(cullingDesc);

    // Forward pass
    if (!mBackbuffer.AcquireNextImage(mImageAcquiredSem))
        LOGE("Failed to acquire next image for rendering");

    ForwardPassDrawDesc forwardDesc;
    forwardDesc.ringBufferPtr = &mRingBuffer;
    forwardDesc.vertexShaderSet = mVertexShaderSet;
    forwardDesc.waitFlags = { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    forwardDesc.waitSems = { mCullingSem, mImageAcquiredSem };
    forwardDesc.signalSem = mRenderSem;
    mForwardPass.Draw(scene, forwardDesc);

    // Particle pass
    ParticlePassDrawDesc particleDesc;
    particleDesc.cameraPos = camera.GetPosition();
    particleDesc.particleDataBuffer = mParticleEngine.GetParticleDataBuffer();
    particleDesc.emitterDataBuffer = mParticleEngine.GetEmitterDataBuffer();
    particleDesc.emitterCount = scene.GetEmitterCount();
    particleDesc.simulationFinishedFence = mParticleEngineFence;
    particleDesc.waitFlags = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    particleDesc.waitSems = { mParticleEngineSem, mRenderSem };
    particleDesc.signalSem = mParticlePassSem;
    particleDesc.fence = mFrameFence;
    mParticlePass.Draw(particleDesc);

    mRingBuffer.MarkFinishedFrame();

    if (!mBackbuffer.Present(mForwardPass.GetTargetTexture(), mParticlePassSem))
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
