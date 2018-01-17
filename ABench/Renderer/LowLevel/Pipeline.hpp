#pragma once

#include "Prerequisites.hpp"
#include "Common/Common.hpp"
#include "Device.hpp"
#include "Shader.hpp"
#include "VertexLayout.hpp"


namespace ABench {
namespace Renderer {

struct GraphicsPipelineDesc
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
    bool enableDepthWrite;
    bool enableColor;

    GraphicsPipelineDesc()
        : vertexShader(nullptr)
        , tessControlShader(nullptr)
        , tessEvalShader(nullptr)
        , geometryShader(nullptr)
        , fragmentShader(nullptr)
        , vertexLayout(nullptr)
        , topology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
        , renderPass(VK_NULL_HANDLE)
        , pipelineLayout(VK_NULL_HANDLE)
        , flags(0)
        , basePipeline(VK_NULL_HANDLE)
        , enableDepth(false)
        , enableDepthWrite(false)
        , enableColor(false)
    {
    }
};

struct ComputePipelineDesc
{
    Shader* computeShader;

    VkPipelineLayout pipelineLayout;
    VkPipelineCreateFlags flags;
    VkPipeline basePipeline;

    ComputePipelineDesc()
        : computeShader(nullptr)
        , pipelineLayout(VK_NULL_HANDLE)
        , flags(0)
        , basePipeline(VK_NULL_HANDLE)
    {
    }
};

class Pipeline
{
    friend class CommandBuffer;

    DevicePtr mDevice;

    VkPipeline mPipeline;

    void BuildShaderStageInfo(const GraphicsPipelineDesc& desc, std::vector<VkPipelineShaderStageCreateInfo>& stages);
    VkPipelineVertexInputStateCreateInfo BuildVertexInputStateInfo(const GraphicsPipelineDesc& desc);
    VkPipelineInputAssemblyStateCreateInfo BuildInputAssemblyStateInfo(const GraphicsPipelineDesc& desc);
    VkPipelineTessellationStateCreateInfo BuildTessellationStateInfo();
    VkPipelineViewportStateCreateInfo BuildViewportStateInfo();
    VkPipelineRasterizationStateCreateInfo BuildRasterizationStateInfo();
    VkPipelineMultisampleStateCreateInfo BuildMultisampleStateInfo();
    VkPipelineDepthStencilStateCreateInfo BuildDepthStencilStateInfo(const GraphicsPipelineDesc& desc);

public:
    Pipeline();
    ~Pipeline();

    bool Init(const DevicePtr& device, const GraphicsPipelineDesc& desc);
    bool Init(const DevicePtr& device, const ComputePipelineDesc& desc);

    ABENCH_INLINE VkPipeline GetPipeline() const
    {
        return mPipeline;
    }
};

using PipelinePtr = std::shared_ptr<Pipeline>;

} // namespace Renderer
} // namespace ABench
