#include "PCH.hpp"
#include "ForwardPass.hpp"

#include "Renderer/LowLevel/DescriptorAllocator.hpp"
#include "Math/Matrix.hpp"

#include "ShaderMacroDefinitions.hpp"


namespace {

struct FragmentParamsCBuffer
{
    uint32_t viewportWidth;
    uint32_t viewportHeight;
    uint32_t pixelsPerGridFrustum;
};

struct MaterialCBuffer
{
    ABench::Math::Vector4 color;
};

} // namespace


namespace ABench {
namespace Renderer {

ForwardPass::ForwardPass()
    : mDevice()
    , mTargetTexture()
    , mDepthTexture(nullptr)
    , mFragmentParams()
    , mFramebuffer()
    , mVertexLayout()
    , mPipeline()
    , mCommandBuffer()
    , mSampler()
    , mFragmentShaderLayout()
    , mFragmentShaderTextureLayout()
    , mRenderPass()
    , mPipelineLayout()
    , mFragmentShaderSet(VK_NULL_HANDLE)
{
}

VkDescriptorSet ForwardPass::AcquireDescriptorSetFromTexture(const TexturePtr& tex)
{
    VkDescriptorSet set = tex->GetDescriptorSet();
    if (set == VK_NULL_HANDLE)
    {
        tex->AllocateDescriptorSet(mFragmentShaderTextureLayout);
        set = tex->GetDescriptorSet();
    }

    return set;
}

bool ForwardPass::Init(const DevicePtr& device, const ForwardPassDesc& desc)
{
    mDevice = device;

    if (desc.depthTexture == nullptr)
    {
        LOGE("Forward pass needs a pregenerated depth texture to work.");
        return false;
    }

    if (desc.depthTexture->GetWidth() != desc.width ||
        desc.depthTexture->GetHeight() != desc.height)
    {
        LOGE("Depth texture dimensions does not match declared output dimensions.");
        return false;
    }

    mDepthTexture = desc.depthTexture;

    TextureDesc targetTexDesc;
    targetTexDesc.width = desc.width;
    targetTexDesc.height = desc.height;
    targetTexDesc.format = desc.outputFormat;
    targetTexDesc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (!mTargetTexture.Init(mDevice, targetTexDesc))
        return false;

    FragmentParamsCBuffer fpBuf;
    fpBuf.viewportWidth = desc.width;
    fpBuf.viewportHeight = desc.height;
    fpBuf.pixelsPerGridFrustum = desc.pixelsPerGridFrustum;

    BufferDesc bufDesc;
    bufDesc.data = &fpBuf;
    bufDesc.dataSize = sizeof(FragmentParamsCBuffer);
    bufDesc.concurrent = false;
    bufDesc.type = BufferType::Static;
    bufDesc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (!mFragmentParams.Init(mDevice, bufDesc))
        return false;


    // Render pass
    std::vector<VkAttachmentDescription> attachments;
    attachments.push_back(Tools::CreateAttachmentDescription(
        desc.outputFormat, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
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

    FramebufferDesc fbDesc;
    fbDesc.colorTex = &mTargetTexture;
    fbDesc.depthTex = mDepthTexture;
    fbDesc.renderPass = mRenderPass;
    if (!mFramebuffer.Init(mDevice, fbDesc))
        return false;


    // Vertex Layout
    VertexLayoutDesc vlDesc;
    std::vector<VertexLayoutEntry> vlEntries;
    vlEntries.emplace_back(VK_FORMAT_R32G32B32_SFLOAT, 0, 0, 12, false); // vertex position
    vlEntries.emplace_back(VK_FORMAT_R32G32B32_SFLOAT, 0, 1, 32, false); // vertex normal
    vlEntries.emplace_back(VK_FORMAT_R32G32_SFLOAT, 12, 1, 32, false); // vertex uv
    vlEntries.emplace_back(VK_FORMAT_R32G32B32_SFLOAT, 20, 1, 32, false); // vertex tangent

    vlDesc.entryCount = static_cast<uint32_t>(vlEntries.size());
    vlDesc.entries = vlEntries.data();
    if (!mVertexLayout.Init(vlDesc))
        return false;


    // Shader layouts and sets
    std::vector<DescriptorSetLayoutDesc> fsLayoutDesc;
    fsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE});
    fsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE});
    fsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE});
    fsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE});
    fsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE});
    mFragmentShaderLayout = Tools::CreateDescriptorSetLayout(mDevice, fsLayoutDesc);
    if (!mFragmentShaderLayout)
        return false;

    mSampler = Tools::CreateSampler(mDevice);
    if (!mSampler)
        return false;

    mFragmentShaderTextureLayout = Tools::CreateDescriptorSetLayout(mDevice,
        { {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, mSampler} });
    if (!mFragmentShaderTextureLayout)
        return false;


    mFragmentShaderSet = DescriptorAllocator::Instance().AllocateDescriptorSet(mFragmentShaderLayout);
    if (mFragmentShaderSet == VK_NULL_HANDLE)
        return false;


    std::vector<VkDescriptorSetLayout> layouts;
    layouts.push_back(desc.vertexShaderLayout);
    layouts.push_back(mFragmentShaderLayout);
    layouts.push_back(mFragmentShaderTextureLayout);
    layouts.push_back(mFragmentShaderTextureLayout);
    layouts.push_back(mFragmentShaderTextureLayout);
    mPipelineLayout = Tools::CreatePipelineLayout(mDevice, layouts);
    if (!mPipelineLayout)
        return false;


    GraphicsPipelineDesc pipeDesc;
    pipeDesc.vertexLayout = &mVertexLayout;
    pipeDesc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipeDesc.renderPass = mRenderPass;
    pipeDesc.pipelineLayout = mPipelineLayout;
    pipeDesc.enableDepth = true;
    pipeDesc.enableDepthWrite = false;
    pipeDesc.enableColor = true;

    MultiGraphicsPipelineDesc mgpDesc;
    mgpDesc.vertexShader.path = "ForwardPass.vert";
    mgpDesc.vertexShader.macros = {
        { ShaderMacro::HAS_NORMAL, 1 },
    };
    mgpDesc.fragmentShader.path = "ForwardPass.frag";
    mgpDesc.fragmentShader.macros = {
        { ShaderMacro::HAS_TEXTURE, 1 },
        { ShaderMacro::HAS_NORMAL, 1 },
        { ShaderMacro::HAS_COLOR_MASK, 1 },
    };
    mgpDesc.pipelineDesc = pipeDesc;

    if (!mPipeline.Init(mDevice, mgpDesc))
        return false;


    if (!mCommandBuffer.Init(mDevice, DeviceQueueType::GRAPHICS))
        return false;

    Tools::UpdateBufferDescriptorSet(mDevice, mFragmentShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
                                     mFragmentParams.GetBuffer(), mFragmentParams.GetSize());
    Tools::UpdateBufferDescriptorSet(mDevice, mFragmentShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1,
                                     desc.ringBufferPtr->GetBuffer(), sizeof(MaterialCBuffer));
    Tools::UpdateBufferDescriptorSet(mDevice, mFragmentShaderSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2,
                                     desc.lightContainerPtr->GetBuffer(), desc.lightContainerPtr->GetSize());
    Tools::UpdateBufferDescriptorSet(mDevice, mFragmentShaderSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3,
                                     desc.culledLightsPtr->GetBuffer(), desc.culledLightsPtr->GetSize());
    Tools::UpdateBufferDescriptorSet(mDevice, mFragmentShaderSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4,
                                     desc.gridLightDataPtr->GetBuffer(), desc.gridLightDataPtr->GetSize());

    return true;
}

void ForwardPass::Draw(const Scene::Scene& scene, const ForwardPassDrawDesc& desc)
{
    ASSERT(desc.waitFlags.size() == desc.waitSems.size(), "Wait semaphores count does not match wait flags count");

    MaterialCBuffer materialBuf;

    // updating buffers
    {
        mCommandBuffer.Begin();

        mCommandBuffer.SetViewport(0, 0, mTargetTexture.GetWidth(), mTargetTexture.GetHeight(), 0.0f, 1.0f);
        mCommandBuffer.SetScissor(0, 0, mTargetTexture.GetWidth(), mTargetTexture.GetHeight());

        float clearValue[] = {0.1f, 0.1f, 0.1f, 0.0f};
        VkPipelineBindPoint bindPoint =  VK_PIPELINE_BIND_POINT_GRAPHICS;
        mCommandBuffer.BeginRenderPass(mRenderPass, &mFramebuffer, ABENCH_CLEAR_COLOR, clearValue, 0.0f);

        MultiGraphicsPipelineShaderMacros macros;
        macros.vertexShader = {
            { ShaderMacro::HAS_NORMAL, 0 },
        };
        macros.fragmentShader = {
            { ShaderMacro::HAS_TEXTURE, 0 },
            { ShaderMacro::HAS_NORMAL, 0 },
            { ShaderMacro::HAS_COLOR_MASK, 0 },
        };

        scene.ForEachObject([&](const Scene::Object* o) -> bool {
            if (o->GetComponent()->GetType() == Scene::ComponentType::Model)
            {
                Scene::Model* model = dynamic_cast<Scene::Model*>(o->GetComponent());

                if (!model->ToRender())
                    return true;

                // world matrix update
                uint32_t offset = desc.ringBufferPtr->Write(&model->GetTransform(), sizeof(ABench::Math::Matrix));
                mCommandBuffer.BindDescriptorSet(desc.vertexShaderSet, bindPoint, 0, mPipelineLayout, offset);

                model->ForEachMesh([&](Scene::Mesh* mesh) {
                    macros.vertexShader[0].value = 0;
                    macros.fragmentShader[0].value = 0;
                    macros.fragmentShader[1].value = 0;
                    macros.fragmentShader[2].value = 0;

                    const Scene::Material* material = mesh->GetMaterial();
                    if (material != nullptr)
                    {
                        // material data update
                        materialBuf.color = material->GetColor();
                        offset = desc.ringBufferPtr->Write(&materialBuf, sizeof(materialBuf));
                        mCommandBuffer.BindDescriptorSet(mFragmentShaderSet, bindPoint, 1, mPipelineLayout, offset);

                        if (material->GetDiffuse())
                        {
                            macros.fragmentShader[0].value = 1;
                            mCommandBuffer.BindDescriptorSet(AcquireDescriptorSetFromTexture(material->GetDiffuse()), bindPoint, 2, mPipelineLayout);
                        }

                        if (material->GetNormal())
                        {
                            macros.vertexShader[0].value = 1;
                            macros.fragmentShader[1].value = 1;
                            mCommandBuffer.BindDescriptorSet(AcquireDescriptorSetFromTexture(material->GetNormal()), bindPoint, 3, mPipelineLayout);
                        }

                        if (material->GetMask())
                        {
                            macros.fragmentShader[2].value = 1;
                            mCommandBuffer.BindDescriptorSet(AcquireDescriptorSetFromTexture(material->GetMask()), bindPoint, 4, mPipelineLayout);
                        }
                    }

                    mCommandBuffer.BindPipeline(mPipeline.GetGraphicsPipeline(macros), bindPoint);
                    mCommandBuffer.BindVertexBuffer(mesh->GetVertexBuffer(), 0, 0);
                    mCommandBuffer.BindVertexBuffer(mesh->GetVertexParamsBuffer(), 1, 0);

                    if (mesh->ByIndices())
                    {
                        mCommandBuffer.BindIndexBuffer(mesh->GetIndexBuffer());
                        mCommandBuffer.DrawIndexed(mesh->GetPointCount());
                    }
                    else
                    {
                        mCommandBuffer.Draw(mesh->GetPointCount(), 1);
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

    mDevice->Execute(DeviceQueueType::GRAPHICS, &mCommandBuffer,
                     static_cast<uint32_t>(desc.waitSems.size()),
                     desc.waitFlags.data(), desc.waitSems.data(), desc.signalSem, desc.fence);
}

} // namespace Renderer
} // namespace ABench
