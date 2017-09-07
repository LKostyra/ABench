#pragma once

#include "Prerequisites.hpp"
#include "Common/Common.hpp"
#include "Device.hpp"
#include "Shader.hpp"
#include "VertexLayout.hpp"


namespace ABench {
namespace Renderer {

struct PipelineDesc
{
    Shader* vertexShader;
    Shader* tessControlShader;
    Shader* tessEvalShader;
    Shader* geometryShader;
    Shader* fragmentShader;

    VertexLayout* vertexLayout;
    VkPrimitiveTopology topology;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipelineCreateFlags flags;
    VkPipeline basePipeline;

    bool enableDepth;

    PipelineDesc()
        : vertexShader(nullptr)
        , tessControlShader(nullptr)
        , tessEvalShader(nullptr)
        , geometryShader(nullptr)
        , fragmentShader(nullptr)
        , vertexLayout(nullptr)
        , topology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
        , renderPass(VK_NULL_HANDLE)
        , pipelineLayout(VK_NULL_HANDLE)
        , enableDepth(false)
        , flags(0)
        , basePipeline(VK_NULL_HANDLE)
    {
    }
};

class Pipeline
{
    friend class CommandBuffer;

    VkPipeline mPipeline;

    void BuildShaderStageInfo(const PipelineDesc& desc, std::vector<VkPipelineShaderStageCreateInfo>& stages);
    VkPipelineVertexInputStateCreateInfo BuildVertexInputStateInfo(const PipelineDesc& desc);
    VkPipelineInputAssemblyStateCreateInfo BuildInputAssemblyStateInfo(const PipelineDesc& desc);
    VkPipelineTessellationStateCreateInfo BuildTessellationStateInfo();
    VkPipelineViewportStateCreateInfo BuildViewportStateInfo();
    VkPipelineRasterizationStateCreateInfo BuildRasterizationStateInfo();
    VkPipelineMultisampleStateCreateInfo BuildMultisampleStateInfo();
    VkPipelineDepthStencilStateCreateInfo BuildDepthStencilStateInfo(const PipelineDesc& desc);

public:
    Pipeline();
    ~Pipeline();

    bool Init(const PipelineDesc& desc);

    ABENCH_INLINE VkPipeline GetPipeline() const
    {
        return mPipeline;
    }
};

using PipelinePtr = std::shared_ptr<Pipeline>;

} // namespace Renderer
} // namespace ABench
