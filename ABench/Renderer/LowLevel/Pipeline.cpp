#include "PCH.hpp"
#include "Pipeline.hpp"
#include "Util.hpp"
#include "Extensions.hpp"
#include "Renderer/HighLevel/Renderer.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

Pipeline::Pipeline()
    : mPipeline(VK_NULL_HANDLE)
{
}

Pipeline::~Pipeline()
{
    LOGD("Destroying Pipeline " << std::hex << mPipeline);
    if (mPipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(mDevice->GetDevice(), mPipeline, nullptr);
}

void Pipeline::BuildShaderStageInfo(const GraphicsPipelineDesc& desc, std::vector<VkPipelineShaderStageCreateInfo>& stages)
{
    VkPipelineShaderStageCreateInfo stage;
    ZERO_MEMORY(stage);
    stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage.pName = "main";

    if (desc.vertexShader)
    {
        stage.module = desc.vertexShader->mShaderModule;
        stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages.push_back(stage);
    }

    if (desc.tessControlShader)
    {
        stage.module = desc.tessControlShader->mShaderModule;
        stage.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        stages.push_back(stage);
    }

    if (desc.tessEvalShader)
    {
        stage.module = desc.tessEvalShader->mShaderModule;
        stage.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        stages.push_back(stage);
    }

    if (desc.geometryShader)
    {
        stage.module = desc.geometryShader->mShaderModule;
        stage.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        stages.push_back(stage);
    }

    if (desc.fragmentShader)
    {
        stage.module = desc.fragmentShader->mShaderModule;
        stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages.push_back(stage);
    }
}

VkPipelineVertexInputStateCreateInfo Pipeline::BuildVertexInputStateInfo(const GraphicsPipelineDesc& desc)
{
    VkPipelineVertexInputStateCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    if (desc.vertexLayout)
    {
        info.vertexAttributeDescriptionCount = static_cast<uint32_t>(desc.vertexLayout->mAttributes.size());
        info.pVertexAttributeDescriptions = desc.vertexLayout->mAttributes.data();
        info.vertexBindingDescriptionCount = static_cast<uint32_t>(desc.vertexLayout->mBindings.size());
        info.pVertexBindingDescriptions = desc.vertexLayout->mBindings.data();
    }

    return info;
}

VkPipelineInputAssemblyStateCreateInfo Pipeline::BuildInputAssemblyStateInfo(const GraphicsPipelineDesc& desc)
{
    VkPipelineInputAssemblyStateCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.primitiveRestartEnable = VK_FALSE;
    info.topology = desc.topology;
    return info;
}

VkPipelineTessellationStateCreateInfo Pipeline::BuildTessellationStateInfo()
{
    VkPipelineTessellationStateCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    info.patchControlPoints = 0;
    return info;
}

VkPipelineViewportStateCreateInfo Pipeline::BuildViewportStateInfo()
{
    VkPipelineViewportStateCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    info.viewportCount = 1;
    info.scissorCount = 1;
    // TODO these are dynamic through Command Buffer, maybe it would be faster to bind these to pipeline state
    return info;
}

VkPipelineRasterizationStateCreateInfo Pipeline::BuildRasterizationStateInfo()
{
    VkPipelineRasterizationStateCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.depthClampEnable = VK_FALSE;
    info.rasterizerDiscardEnable = VK_FALSE;
    info.polygonMode = VK_POLYGON_MODE_FILL;
    info.cullMode = VK_CULL_MODE_BACK_BIT;
    info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    info.depthBiasEnable = VK_FALSE;
    info.lineWidth = 1.0f;
    return info;
}

VkPipelineMultisampleStateCreateInfo Pipeline::BuildMultisampleStateInfo()
{
    VkPipelineMultisampleStateCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    info.sampleShadingEnable = VK_FALSE;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable = VK_FALSE;
    return info;
}

VkPipelineDepthStencilStateCreateInfo Pipeline::BuildDepthStencilStateInfo(const GraphicsPipelineDesc& desc)
{
    VkPipelineDepthStencilStateCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.depthTestEnable = desc.enableDepth ? VK_TRUE : VK_FALSE;
    info.depthWriteEnable = desc.enableDepth ? VK_TRUE : VK_FALSE;
    info.depthCompareOp = desc.enableDepth ? VK_COMPARE_OP_LESS : VK_COMPARE_OP_ALWAYS;
    info.depthBoundsTestEnable = false;
    info.stencilTestEnable = VK_FALSE;

    VkStencilOpState stencil;
    ZERO_MEMORY(stencil);
    stencil.failOp = VK_STENCIL_OP_KEEP;
    stencil.passOp = VK_STENCIL_OP_KEEP;
    stencil.depthFailOp = VK_STENCIL_OP_KEEP;
    stencil.compareOp = VK_COMPARE_OP_ALWAYS;
    stencil.compareMask = 0xFF;
    stencil.writeMask = 0xFF;

    info.front = info.back = stencil;

    return info;
}

bool Pipeline::Init(const DevicePtr& device, const GraphicsPipelineDesc& desc)
{
    if (mPipeline != VK_NULL_HANDLE)
    {
        LOGE("Cannot initialize already initialized pipeline");
        return false;
    }

    mDevice = device;

    std::vector<VkPipelineShaderStageCreateInfo> stages;
    BuildShaderStageInfo(desc, stages);
    VkPipelineVertexInputStateCreateInfo vertexInputState = BuildVertexInputStateInfo(desc);
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = BuildInputAssemblyStateInfo(desc);
    VkPipelineTessellationStateCreateInfo tessellationState = BuildTessellationStateInfo();
    VkPipelineViewportStateCreateInfo viewportState = BuildViewportStateInfo();
    VkPipelineRasterizationStateCreateInfo rasterizationState = BuildRasterizationStateInfo();
    VkPipelineMultisampleStateCreateInfo multisampleState = BuildMultisampleStateInfo();
    VkPipelineDepthStencilStateCreateInfo depthStencilState = BuildDepthStencilStateInfo(desc);

    // due to pAttachments, color blend state must be built here
    VkPipelineColorBlendStateCreateInfo colorBlendState;
    ZERO_MEMORY(colorBlendState);
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState attachment;
    ZERO_MEMORY(attachment);
    attachment.colorWriteMask = 0xF;
    attachment.blendEnable = VK_FALSE;
    attachment.colorBlendOp = VK_BLEND_OP_ADD;
    attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    if (desc.enableColor)
    {
        colorBlendState.attachmentCount = 1;
        colorBlendState.pAttachments = &attachment;
    }
    else
    {
        colorBlendState.attachmentCount = 0;
        colorBlendState.pAttachments = nullptr;
    }

    // due to pDynamicStates, dynamic state must be built here
    std::vector<VkDynamicState> dynamicStates;
    dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    ZERO_MEMORY(dynamicStateInfo);
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates = dynamicStates.data();


    // construct the Pipeline
    VkGraphicsPipelineCreateInfo pipeInfo;
    ZERO_MEMORY(pipeInfo);
    pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeInfo.stageCount = static_cast<uint32_t>(stages.size());
    pipeInfo.pStages = stages.data();
    pipeInfo.pVertexInputState = &vertexInputState;
    pipeInfo.pInputAssemblyState = &inputAssemblyState;
    if ((desc.tessEvalShader != nullptr) && (desc.tessControlShader != nullptr))
        pipeInfo.pTessellationState = &tessellationState;
    pipeInfo.pViewportState = &viewportState;
    pipeInfo.pRasterizationState = &rasterizationState;
    pipeInfo.pMultisampleState = &multisampleState;
    pipeInfo.pDepthStencilState = &depthStencilState;
    pipeInfo.pColorBlendState = &colorBlendState;
    pipeInfo.pDynamicState = &dynamicStateInfo;
    pipeInfo.layout = desc.pipelineLayout;
    pipeInfo.renderPass = desc.renderPass;
    pipeInfo.subpass = 0;
    pipeInfo.flags = desc.flags;
    if (desc.flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT)
    {
        pipeInfo.basePipelineHandle = desc.basePipeline;
        pipeInfo.basePipelineIndex = -1;
    }

    VkResult result = vkCreateGraphicsPipelines(mDevice->GetDevice(), VK_NULL_HANDLE, 1,
                                                &pipeInfo, nullptr, &mPipeline);
    RETURN_FALSE_IF_FAILED(result, "Failed to create Graphics Pipeline");

    LOGI("Graphics Pipeline created successfully");
    return true;
}

bool Pipeline::Init(const DevicePtr& device, const ComputePipelineDesc& desc)
{
    if (mPipeline != VK_NULL_HANDLE)
    {
        LOGE("Cannot initialize already initialized Pipeline");
        return false;
    }

    if (desc.computeShader == nullptr)
    {
        LOGE("Compute shader is required to create Compute Pipeline");
        return false;
    }

    mDevice = device;

    VkPipelineShaderStageCreateInfo stage;
    ZERO_MEMORY(stage);
    stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage.pName = "main";
    stage.module = desc.computeShader->mShaderModule;
    stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;

    VkComputePipelineCreateInfo pipeInfo;
    ZERO_MEMORY(pipeInfo);
    pipeInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeInfo.stage = stage;
    pipeInfo.layout = desc.pipelineLayout;
    pipeInfo.flags = desc.flags;
    if (desc.flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT)
    {
        pipeInfo.basePipelineHandle = desc.basePipeline;
        pipeInfo.basePipelineIndex = -1;
    }

    VkResult result = vkCreateComputePipelines(mDevice->GetDevice(), VK_NULL_HANDLE, 1,
                                               &pipeInfo, nullptr, &mPipeline);
    RETURN_FALSE_IF_FAILED(result, "Failed to create Compute Pipeline");

    LOGI("Compute Pipeline created successfully");
    return true;
}

} // namespace Renderer
} // namespace ABench
