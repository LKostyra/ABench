#include "../PCH.hpp"
#include "Pipeline.hpp"
#include "Util.hpp"
#include "Extensions.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

Pipeline::Pipeline(const Device* device)
    : mDevicePtr(device)
{
}

Pipeline::~Pipeline()
{
}

std::vector<VkPipelineShaderStageCreateInfo> Pipeline::BuildShaderStageInfo(const PipelineDesc& desc)
{
    std::vector<VkPipelineShaderStageCreateInfo> stages;
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

    return stages;
}

VkPipelineVertexInputStateCreateInfo Pipeline::BuildVertexInputStateInfo(const PipelineDesc& desc)
{
    VkPipelineVertexInputStateCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.vertexAttributeDescriptionCount = static_cast<uint32_t>(desc.vertexLayout->mAttributes.size());
    info.pVertexAttributeDescriptions = desc.vertexLayout->mAttributes.data();
    info.vertexBindingDescriptionCount = static_cast<uint32_t>(desc.vertexLayout->mBindings.size());
    info.pVertexBindingDescriptions = desc.vertexLayout->mBindings.data();
    return info;
}

VkPipelineInputAssemblyStateCreateInfo Pipeline::BuildInputAssemblyStateInfo(const PipelineDesc& desc)
{
    VkPipelineInputAssemblyStateCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.primitiveRestartEnable = VK_FALSE;
    info.topology = desc.topology;
    return info;
}

bool Pipeline::Init(const PipelineDesc& desc)
{
    std::vector<VkPipelineShaderStageCreateInfo> stages = BuildShaderStageInfo(desc);
    VkPipelineVertexInputStateCreateInfo vertexInputState = BuildVertexInputStateInfo(desc);
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = BuildInputAssemblyStateInfo(desc);

    VkGraphicsPipelineCreateInfo pipeInfo;
    ZERO_MEMORY(pipeInfo);
    pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeInfo.stageCount = static_cast<uint32_t>(stages.size());
    pipeInfo.pStages = stages.data();
    pipeInfo.pVertexInputState = &vertexInputState;
    pipeInfo.pInputAssemblyState = &inputAssemblyState;
    pipeInfo.pTessellationState = nullptr; // skipped, no Tessellation needed
    pipeInfo.
    pipeInfo.renderPass = desc.renderPass->mRenderPass;
    VkResult result = vkCreateGraphicsPipelines(mDevicePtr->mDevice, mDevicePtr->mPipelineCache, 1,
                                                &pipeInfo, nullptr, &mPipeline);
    CHECK_VKRESULT(result, "Failed to create a Graphics Pipeline");

    LOGI("Graphics Pipeline created successfully");
    return true;
}

} // namespace Renderer
} // namespace ABench
