#include "PCH.hpp"
#include "DepthPrePass.hpp"

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


} // namespace


namespace ABench {
namespace Renderer {

DepthPrePass::DepthPrePass()
    : mRenderPass()
    , mPipelineLayout()
    , mVertexShaderSet(VK_NULL_HANDLE)
{
}

bool DepthPrePass::Init(const DevicePtr& device, const DepthPrePassDesc& desc)
{
    mDevice = device;

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
        VK_FORMAT_D32_SFLOAT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    ));

    std::vector<VkAttachmentReference> colorAttRefs; // left empty, we only want depth buffer out of this pass
    VkAttachmentReference depthAttRef = Tools::CreateAttachmentReference(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    std::vector<VkSubpassDescription> subpasses;
    subpasses.push_back(Tools::CreateSubpass(colorAttRefs, &depthAttRef));

    std::vector<VkSubpassDependency> subpassDeps;
    subpassDeps.push_back(Tools::CreateSubpassDependency(
        VK_SUBPASS_EXTERNAL, 0,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
    ));
    subpassDeps.push_back(Tools::CreateSubpassDependency(
        0, VK_SUBPASS_EXTERNAL,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT
    ));

    mRenderPass = Tools::CreateRenderPass(mDevice, attachments, subpasses, subpassDeps);
    if (!mRenderPass)
        return false;

    FramebufferDesc fbDesc;
    fbDesc.colorTex = nullptr;
    fbDesc.depthTex = &mDepthTexture;
    fbDesc.renderPass = mRenderPass;
    if (!mFramebuffer.Init(mDevice, fbDesc))
        return false;

    VertexLayoutDesc vlDesc;

    std::vector<VertexLayoutEntry> vlEntries;
    vlEntries.emplace_back(VK_FORMAT_R32G32B32_SFLOAT, 0, 0, 12, false); // vertex position
    vlDesc.entryCount = static_cast<uint32_t>(vlEntries.size());
    vlDesc.entries = vlEntries.data();
    if (!mVertexLayout.Init(vlDesc))
        return false;

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.push_back(DescriptorLayoutManager::Instance().GetVertexShaderLayout());
    mPipelineLayout = Tools::CreatePipelineLayout(mDevice, layouts);
    if (!mPipelineLayout)
        return false;

    // buffer-related set allocation
    mVertexShaderSet = DescriptorAllocator::Instance().AllocateDescriptorSet(DescriptorLayoutManager::Instance().GetVertexShaderLayout());
    if (mVertexShaderSet == VK_NULL_HANDLE)
        return false;


    GraphicsPipelineDesc pipeDesc;
    pipeDesc.vertexLayout = &mVertexLayout;
    pipeDesc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipeDesc.renderPass = mRenderPass;
    pipeDesc.pipelineLayout = mPipelineLayout;
    pipeDesc.enableDepth = true;
    pipeDesc.enableDepthWrite = true;
    pipeDesc.enableColor = false;

    MultiGraphicsPipelineDesc mgpDesc;
    mgpDesc.vertexShader.path = "DepthPrePass.vert";
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


    // Point vertex shader set bindings to our dynamic buffer
    Tools::UpdateBufferDescriptorSet(mDevice, mVertexShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0,
                                     mRingBuffer.GetVkBuffer(), sizeof(VertexShaderDynamicCBuffer));
    Tools::UpdateBufferDescriptorSet(mDevice, mVertexShaderSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                                     mVertexShaderCBuffer.GetBuffer(), sizeof(VertexShaderCBuffer));

    return true;
}

void DepthPrePass::Draw(const Scene::Scene& scene, const Scene::Camera& camera, VkSemaphore signalSem, VkFence fence)
{
    // Update viewport
    // TODO view could be pushed to dynamic buffer for optimization
    VertexShaderCBuffer buf;
    buf.viewMatrix = camera.GetView();
    buf.projMatrix = camera.GetProjection();

    // updating buffers
    mVertexShaderCBuffer.Write(&buf, sizeof(VertexShaderCBuffer));

    {
        mCommandBuffer.Begin();

        mCommandBuffer.SetViewport(0, 0, mDepthTexture.GetWidth(), mDepthTexture.GetHeight(), 0.0f, 1.0f);
        mCommandBuffer.SetScissor(0, 0, mDepthTexture.GetWidth(), mDepthTexture.GetHeight());

        VkPipelineBindPoint bindPoint =  VK_PIPELINE_BIND_POINT_GRAPHICS;
        mCommandBuffer.BeginRenderPass(mRenderPass, &mFramebuffer, ABENCH_CLEAR_DEPTH, nullptr, 1.0f);

        MultiGraphicsPipelineShaderMacros emptyMacros;
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
                    mCommandBuffer.BindPipeline(mPipeline.GetGraphicsPipeline(emptyMacros), bindPoint);
                    mCommandBuffer.BindVertexBuffer(mesh->GetVertexBuffer(), 0);

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

    mDevice->Execute(DeviceQueueType::GRAPHICS, &mCommandBuffer, 0,
                     nullptr, nullptr, signalSem, fence);

    mRingBuffer.MarkFinishedFrame();
}

} // namespace Renderer
} // namespace ABench
