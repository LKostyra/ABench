#include "PCH.hpp"
#include "ParticlePass.hpp"
#include "Renderer/LowLevel/DescriptorAllocator.hpp"
#include "Renderer/LowLevel/Translations.hpp"
#include "Renderer/LowLevel/Extensions.hpp"
#include "Common/Image.hpp"


namespace ABench {
namespace Renderer {

ParticlePass::ParticlePass()
    : mDevice()
{
}

bool ParticlePass::Init(const DevicePtr& device, const ParticlePassDesc& desc)
{
    mDevice = device;

    float particle[] = { -0.005f,-0.005f, 0.0f, 0.0f, 0.0f,
                         -0.005f, 0.005f, 0.0f, 0.0f, 1.0f,
                          0.005f,-0.005f, 0.0f, 1.0f, 0.0f,
                         -0.005f, 0.005f, 0.0f, 0.0f, 1.0f,
                          0.005f, 0.005f, 0.0f, 1.0f, 1.0f,
                          0.005f,-0.005f, 0.0f, 1.0f, 0.0f, };

    BufferDesc pvbDesc;
    pvbDesc.data = particle;
    pvbDesc.dataSize = sizeof(particle);
    pvbDesc.concurrent = false;
    pvbDesc.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    pvbDesc.type = BufferType::Static;
    if (!mParticleVertexBuffer.Init(mDevice, pvbDesc))
        return false;

    Common::Image particleImage;
    if (!particleImage.Init(desc.particleTexturePath, true))
    {
        LOGE("Failed to initialize particle system - couldn't load particle texture " << desc.particleTexturePath);
        return false;
    }

    std::vector<Renderer::TextureDataDesc> textures;
    textures.reserve(particleImage.GetMipmapCount());
    for (uint32_t i = 0; i < particleImage.GetMipmapCount(); ++i)
        textures.emplace_back(particleImage.GetSubimageData(i), particleImage.GetSubimageSize(i));

    Renderer::TextureDesc texDesc;
    texDesc.width = particleImage.GetWidth();
    texDesc.height = particleImage.GetHeight();
    texDesc.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    texDesc.data = textures.data();
    texDesc.mipmapCount = static_cast<uint32_t>(textures.size());
    texDesc.format = VK_FORMAT_B8G8R8A8_UNORM;
    texDesc.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    if (!mParticleTexture.Init(mDevice, texDesc))
        return false;


    // Render pass
    std::vector<VkAttachmentDescription> attachments;
    attachments.push_back(Tools::CreateAttachmentDescription(
        desc.outputFormat, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    ));
    attachments.push_back(Tools::CreateAttachmentDescription(
        VK_FORMAT_D32_SFLOAT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    ));

    std::vector<VkAttachmentReference> colorAttRefs;
    colorAttRefs.push_back(Tools::CreateAttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
    VkAttachmentReference depthAttRef = Tools::CreateAttachmentReference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    std::vector<VkSubpassDescription> subpasses;
    subpasses.push_back(Tools::CreateSubpass(colorAttRefs, &depthAttRef));

    std::vector<VkSubpassDependency> subpassDeps;
    subpassDeps.push_back(Tools::CreateSubpassDependency(
        VK_SUBPASS_EXTERNAL, 0,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    ));
    subpassDeps.push_back(Tools::CreateSubpassDependency(
        0, VK_SUBPASS_EXTERNAL,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT
    ));

    mRenderPass = Tools::CreateRenderPass(mDevice, attachments, subpasses, subpassDeps);
    if (!mRenderPass)
        return false;


    // Framebuffer
    FramebufferDesc fbDesc;
    fbDesc.colorTex = desc.targetTexture;
    fbDesc.depthTex = desc.depthTexture;
    fbDesc.renderPass = mRenderPass;
    if (!mFramebuffer.Init(mDevice, fbDesc))
        return false;


    // Vertex Layout
    VertexLayoutDesc vlDesc;
    std::vector<VertexLayoutEntry> vlEntries;
    vlEntries.emplace_back(VK_FORMAT_R32G32B32_SFLOAT, 0, 0, 20, false); // vertex position
    vlEntries.emplace_back(VK_FORMAT_R32G32_SFLOAT, 12, 0, 20, false); // vertex uv
    vlEntries.emplace_back(VK_FORMAT_R32G32B32A32_SFLOAT, 0, 1, 64, true); // instance pos
    vlEntries.emplace_back(VK_FORMAT_R32G32B32_SFLOAT, 16, 1, 64, true); // instance color
    vlEntries.emplace_back(VK_FORMAT_R32_SFLOAT, 28, 1, 64, true); // instance lifeTimer
    vlEntries.emplace_back(VK_FORMAT_R32G32B32_SFLOAT, 48, 1, 64, true); // instance velocity
    vlDesc.entryCount = static_cast<uint32_t>(vlEntries.size());
    vlDesc.entries = vlEntries.data();
    if (!mVertexLayout.Init(vlDesc))
        return false;


    std::vector<DescriptorSetLayoutDesc> vsLayoutDesc;
    vsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_NULL_HANDLE});
    mVertexShaderSetLayout = Tools::CreateDescriptorSetLayout(mDevice, vsLayoutDesc);
    if (!mVertexShaderSetLayout)
        return false;

    mVertexShaderSet = DescriptorAllocator::Instance().AllocateDescriptorSet(mVertexShaderSetLayout);
    if (mVertexShaderSet == VK_NULL_HANDLE)
        return false;

    mSampler = Tools::CreateSampler(mDevice);
    if (!mSampler)
        return false;

    std::vector<DescriptorSetLayoutDesc> fsLayoutDesc;
    fsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, mSampler});
    mFragmentShaderSetLayout = Tools::CreateDescriptorSetLayout(mDevice, fsLayoutDesc);
    if (!mFragmentShaderSetLayout)
        return false;

    mFragmentShaderSet = DescriptorAllocator::Instance().AllocateDescriptorSet(mFragmentShaderSetLayout);
    if (mFragmentShaderSet == VK_NULL_HANDLE)
        return false;

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.push_back(mVertexShaderSetLayout);
    layouts.push_back(mFragmentShaderSetLayout);
    mPipelineLayout = Tools::CreatePipelineLayout(mDevice, layouts);
    if (!mPipelineLayout)
        return false;


    // Pipeline
    GraphicsPipelineDesc pipeDesc;
    pipeDesc.vertexLayout = &mVertexLayout;
    pipeDesc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipeDesc.renderPass = mRenderPass;
    pipeDesc.pipelineLayout = mPipelineLayout;
    pipeDesc.enableDepth = true;
    pipeDesc.enableDepthWrite = true;
    pipeDesc.enableColor = true;
    pipeDesc.enableColorBlend = true;

    MultiGraphicsPipelineDesc mgpDesc;
    mgpDesc.vertexShader.path = "ParticlePass.vert";
    mgpDesc.fragmentShader.path = "ParticlePass.frag";
    mgpDesc.pipelineDesc = pipeDesc;
    if (!mPipeline.Init(mDevice, mgpDesc))
        return false;


    if (!mCommandBuffer.Init(mDevice, DeviceQueueType::GRAPHICS))
        return false;


    Tools::UpdateBufferDescriptorSet(mDevice, mVertexShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
                                     desc.vertexShaderBuffer->GetBuffer(), desc.vertexShaderBuffer->GetSize());
    Tools::UpdateTextureDescriptorSet(mDevice, mFragmentShaderSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0,
                                      mParticleTexture.GetView());

    return true;
}

void ParticlePass::Draw(const ParticlePassDrawDesc& desc)
{
    std::vector<Scene::EmitterData> emitterData(desc.emitterCount);

    {
        // begin recording basic stuff
        mCommandBuffer.Begin();

        mCommandBuffer.SetViewport(0, 0, mFramebuffer.GetWidth(), mFramebuffer.GetHeight(), 0.0f, 1.0f);
        mCommandBuffer.SetScissor(0, 0, mFramebuffer.GetWidth(), mFramebuffer.GetHeight());

        mCommandBuffer.BufferBarrier(desc.particleDataBuffer, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                                     0, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                                     mDevice->GetQueueIndex(DeviceQueueType::COMPUTE), mDevice->GetQueueIndex(DeviceQueueType::GRAPHICS));

        VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        mCommandBuffer.BeginRenderPass(mRenderPass, &mFramebuffer, ABENCH_CLEAR_NONE, nullptr, 0.0f);

        MultiGraphicsPipelineShaderMacros emptyMacros;

        mCommandBuffer.BindPipeline(mPipeline.GetGraphicsPipeline(emptyMacros), bindPoint);
        mCommandBuffer.BindDescriptorSet(mVertexShaderSet, bindPoint, 0, mPipelineLayout);
        mCommandBuffer.BindDescriptorSet(mFragmentShaderSet, bindPoint, 1, mPipelineLayout);
        mCommandBuffer.BindVertexBuffer(&mParticleVertexBuffer, 0, 0);


        // wait until simulation engine finishes it's job
        // we need it for emitter data, it is not being sorted/processed later
        VkFence fences[] = { desc.simulationFinishedFence };
        VkResult result = vkWaitForFences(mDevice->GetDevice(), 1, fences, VK_TRUE, UINT64_MAX);
        if (result != VK_SUCCESS)
            LOGW("Failed to wait for fence: " << result << " (" << TranslateVkResultToString(result) << ")");

        result = vkResetFences(mDevice->GetDevice(), 1, fences);
        if (result != VK_SUCCESS)
            LOGW("Failed to reset frame fence: " << result << " (" << TranslateVkResultToString(result) << ")");

        // read emitter data
        if (!desc.emitterDataBuffer->Read(emitterData.data(), desc.emitterDataBuffer->GetSize()))
            LOGW("Failed to read emitter data");


        // finish drawing and go on with life
        for (auto& e: emitterData)
        {
            mCommandBuffer.BindVertexBuffer(desc.particleDataBuffer, 1, e.particleDataOffset);
            mCommandBuffer.Draw(6, e.lastSpawnedParticle);
        }

        mCommandBuffer.EndRenderPass();

        mCommandBuffer.BufferBarrier(desc.particleDataBuffer, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                     VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, 0,
                                     mDevice->GetQueueIndex(DeviceQueueType::GRAPHICS), mDevice->GetQueueIndex(DeviceQueueType::COMPUTE));

        if (!mCommandBuffer.End())
        {
            LOGE("Failure during Command Buffer recording");
            return;
        }
    }

    mDevice->Execute(DeviceQueueType::GRAPHICS, &mCommandBuffer,
                     static_cast<uint32_t>(desc.waitSems.size()),
                     desc.waitFlags.data(), desc.waitSems.data(),
                     desc.signalSem, desc.fence);
}

} // namespace Renderer
} // namespace ABench
