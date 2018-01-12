#include "PCH.hpp"
#include "ForwardPass.hpp"

#include "Renderer/LowLevel/DescriptorAllocator.hpp"
#include "Math/Matrix.hpp"

#include "DescriptorLayoutManager.hpp"
#include "ShaderMacroDefinitions.hpp"


namespace {


struct VertexShaderDynamicCBuffer
{
    ABench::Math::Matrix worldMatrix;
};

struct VertexShaderCBuffer
{
    ABench::Math::Matrix viewMatrix;
    ABench::Math::Matrix projMatrix;
};

struct FragmentShaderLightCBuffer
{
    ABench::Math::Vector4 pos;
    ABench::Math::Vector4 diffuse;
};

struct MaterialCBuffer
{
    ABench::Math::Vector4 color;
};


} // namespace


namespace ABench {
namespace Renderer {

ForwardPass::ForwardPass()
    : mRenderPass()
    , mPipelineLayout()
    , mVertexShaderSet(VK_NULL_HANDLE)
    , mFragmentShaderSet(VK_NULL_HANDLE)
    , mAllShaderSet(VK_NULL_HANDLE)
{
}

bool ForwardPass::Init(const DevicePtr& device, const ForwardPassDesc& desc)
{
    mDevice = device;

    TextureDesc targetTexDesc;
    targetTexDesc.width = desc.width;
    targetTexDesc.height = desc.height;
    targetTexDesc.format = desc.outputFormat;
    targetTexDesc.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (!mTargetTexture.Init(mDevice, targetTexDesc))
        return false;

    TextureDesc depthTexDesc;
    depthTexDesc.width = desc.width;
    depthTexDesc.height = desc.height;
    depthTexDesc.format = VK_FORMAT_D32_SFLOAT;
    depthTexDesc.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (!mDepthTexture.Init(mDevice, depthTexDesc))
        return false;

    if (!mRingBuffer.Init(mDevice, 1024*1024)) // 1M ring buffer should be enough
        return false;

    std::vector<VkAttachmentDescription> attachments;
    attachments.push_back(Tools::CreateAttachmentDescription(
        desc.outputFormat, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    ));
    attachments.push_back(Tools::CreateAttachmentDescription(
        VK_FORMAT_D32_SFLOAT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
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
    fbDesc.depthTex = &mDepthTexture;
    fbDesc.renderPass = mRenderPass;
    if (!mFramebuffer.Init(mDevice, fbDesc))
        return false;

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

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.push_back(DescriptorLayoutManager::Instance().GetVertexShaderLayout());
    layouts.push_back(DescriptorLayoutManager::Instance().GetAllShaderLayout());
    layouts.push_back(DescriptorLayoutManager::Instance().GetFragmentShaderDiffuseTextureLayout());
    layouts.push_back(DescriptorLayoutManager::Instance().GetFragmentShaderNormalTextureLayout());
    layouts.push_back(DescriptorLayoutManager::Instance().GetFragmentShaderMaskTextureLayout());
    layouts.push_back(DescriptorLayoutManager::Instance().GetFragmentShaderLayout());
    mPipelineLayout = Tools::CreatePipelineLayout(mDevice, layouts);
    if (!mPipelineLayout)
        return false;

    // buffer-related set allocation
    mVertexShaderSet = DescriptorAllocator::Instance().AllocateDescriptorSet(DescriptorLayoutManager::Instance().GetVertexShaderLayout());
    if (mVertexShaderSet == VK_NULL_HANDLE)
        return false;

    mFragmentShaderSet = DescriptorAllocator::Instance().AllocateDescriptorSet(DescriptorLayoutManager::Instance().GetFragmentShaderLayout());
    if (mFragmentShaderSet == VK_NULL_HANDLE)
        return false;

    mAllShaderSet = DescriptorAllocator::Instance().AllocateDescriptorSet(DescriptorLayoutManager::Instance().GetAllShaderLayout());
    if (mAllShaderSet == VK_NULL_HANDLE)
        return false;


    GraphicsPipelineDesc pipeDesc;
    pipeDesc.vertexLayout = &mVertexLayout;
    pipeDesc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipeDesc.renderPass = mRenderPass;
    pipeDesc.pipelineLayout = mPipelineLayout;
    pipeDesc.enableDepth = true;
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


    BufferDesc vsBufferDesc;
    vsBufferDesc.data = nullptr;
    vsBufferDesc.dataSize = sizeof(VertexShaderCBuffer);
    vsBufferDesc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vsBufferDesc.type = BufferType::Dynamic;
    if (!mVertexShaderCBuffer.Init(mDevice, vsBufferDesc))
        return false;

    BufferDesc fsBufferDesc;
    fsBufferDesc.data = nullptr;
    fsBufferDesc.dataSize = sizeof(FragmentShaderLightCBuffer);
    fsBufferDesc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    fsBufferDesc.type = BufferType::Dynamic;
    if (!mAllShaderLightCBuffer.Init(mDevice, vsBufferDesc))
        return false;

    // Point vertex shader set bindings to our dynamic buffer
    Tools::UpdateBufferDescriptorSet(mDevice, mVertexShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0,
                                     mRingBuffer.GetVkBuffer(), sizeof(VertexShaderDynamicCBuffer));
    Tools::UpdateBufferDescriptorSet(mDevice, mVertexShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                                     mVertexShaderCBuffer.GetBuffer(), sizeof(VertexShaderCBuffer));
    Tools::UpdateBufferDescriptorSet(mDevice, mFragmentShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0,
                                     mRingBuffer.GetVkBuffer(), sizeof(MaterialCBuffer));
    Tools::UpdateBufferDescriptorSet(mDevice, mAllShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
                                     mAllShaderLightCBuffer.GetBuffer(), sizeof(FragmentShaderLightCBuffer));

    return true;
}

void ForwardPass::Draw(const Scene::Scene& scene, const Scene::Camera& camera, VkSemaphore waitSem, VkSemaphore signalSem, VkFence fence)
{
    // Update viewport
    // TODO view could be pushed to dynamic buffer for optimization
    VertexShaderCBuffer buf;
    buf.viewMatrix = camera.GetView();
    buf.projMatrix = camera.GetProjection();

    FragmentShaderLightCBuffer lightBuf;
    MaterialCBuffer materialBuf;

    scene.ForEachLight([&lightBuf](const Scene::Object* o) -> bool {
        // gather only first light's position for now
        Scene::Light* l = dynamic_cast<Scene::Light*>(o->GetComponent());

        lightBuf.pos = o->GetPosition();
        lightBuf.diffuse = l->GetDiffuseIntensity();
        return false;
    });

    // updating buffers
    mVertexShaderCBuffer.Write(&buf, sizeof(VertexShaderCBuffer));
    mAllShaderLightCBuffer.Write(&lightBuf, sizeof(FragmentShaderLightCBuffer));

    {
        mCommandBuffer.Begin();

        mCommandBuffer.SetViewport(0, 0, mTargetTexture.GetWidth(), mTargetTexture.GetHeight(), 0.0f, 1.0f);
        mCommandBuffer.SetScissor(0, 0, mTargetTexture.GetWidth(), mTargetTexture.GetHeight());

        float clearValue[] = {0.1f, 0.1f, 0.1f, 0.0f};
        VkPipelineBindPoint bindPoint =  VK_PIPELINE_BIND_POINT_GRAPHICS;
        mCommandBuffer.BeginRenderPass(mRenderPass, &mFramebuffer, ABENCH_CLEAR_ALL, clearValue, 1.0f);
        mCommandBuffer.BindDescriptorSet(mAllShaderSet, bindPoint, 1, mPipelineLayout);

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
            if (!o->ToRender())
                return true;

            if (o->GetComponent()->GetType() == Scene::ComponentType::Model)
            {
                Scene::Model* model = dynamic_cast<Scene::Model*>(o->GetComponent());

                // world matrix update
                uint32_t offset = mRingBuffer.Write(&o->GetTransform(), sizeof(ABench::Math::Matrix));
                mCommandBuffer.BindDescriptorSet(mVertexShaderSet, bindPoint, 0, mPipelineLayout, offset);

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
                        offset = mRingBuffer.Write(&materialBuf, sizeof(materialBuf));
                        mCommandBuffer.BindDescriptorSet(mFragmentShaderSet, bindPoint, 5, mPipelineLayout, offset);

                        if (material->GetDiffuseDescriptor() != VK_NULL_HANDLE)
                        {
                            macros.fragmentShader[0].value = 1;
                            mCommandBuffer.BindDescriptorSet(material->GetDiffuseDescriptor(), bindPoint, 2, mPipelineLayout);
                        }

                        if (material->GetNormalDescriptor() != VK_NULL_HANDLE)
                        {
                            macros.vertexShader[0].value = 1;
                            macros.fragmentShader[1].value = 1;
                            mCommandBuffer.BindDescriptorSet(material->GetNormalDescriptor(), bindPoint, 3, mPipelineLayout);
                        }

                        if (material->GetMaskDescriptor() != VK_NULL_HANDLE)
                        {
                            macros.fragmentShader[2].value = 1;
                            mCommandBuffer.BindDescriptorSet(material->GetMaskDescriptor(), bindPoint, 4, mPipelineLayout);
                        }
                    }

                    mCommandBuffer.BindPipeline(mPipeline.GetGraphicsPipeline(macros), bindPoint);
                    mCommandBuffer.BindVertexBuffer(mesh->GetVertexBuffer(), 0);
                    mCommandBuffer.BindVertexBuffer(mesh->GetVertexParamsBuffer(), 1);

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

    mDevice->Execute(DeviceQueueType::GRAPHICS, &mCommandBuffer, waitSem, signalSem, fence);

    mRingBuffer.MarkFinishedFrame();
}

} // namespace Renderer
} // namespace ABench
