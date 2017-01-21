#pragma once

#include "Device.hpp"
#include "Shader.hpp"
#include "VertexLayout.hpp"
#include "RenderPass.hpp"
#include "PipelineLayout.hpp"


namespace ABench {
namespace Renderer {

// TODO create Prerequisites.hpp
class Shader;
class PipelineLayout;

struct PipelineDesc
{
    Shader* vertexShader;
    Shader* tessControlShader;
    Shader* tessEvalShader;
    Shader* geometryShader;
    Shader* fragmentShader;

    VertexLayout* vertexLayout;
    VkPrimitiveTopology topology;
    RenderPass* renderPass;
    PipelineLayout* pipelineLayout;

    PipelineDesc()
        : vertexShader(nullptr)
        , tessControlShader(nullptr)
        , tessEvalShader(nullptr)
        , geometryShader(nullptr)
        , fragmentShader(nullptr)
        , vertexLayout(nullptr)
        , topology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
        , renderPass(nullptr)
        , pipelineLayout(nullptr)
    {
    }
};

class Pipeline
{
    friend class CommandBuffer;

    const Device* mDevicePtr;

    VkPipeline mPipeline;

    void BuildShaderStageInfo(const PipelineDesc& desc, std::vector<VkPipelineShaderStageCreateInfo>& stages);
    VkPipelineVertexInputStateCreateInfo BuildVertexInputStateInfo(const PipelineDesc& desc);
    VkPipelineInputAssemblyStateCreateInfo BuildInputAssemblyStateInfo(const PipelineDesc& desc);
    VkPipelineTessellationStateCreateInfo BuildTessellationStateInfo(const PipelineDesc& desc);
    VkPipelineViewportStateCreateInfo BuildViewportStateInfo(const PipelineDesc& desc);
    VkPipelineRasterizationStateCreateInfo BuildRasterizationStateInfo(const PipelineDesc& desc);
    VkPipelineMultisampleStateCreateInfo BuildMultisampleStateInfo(const PipelineDesc& desc);
    VkPipelineDepthStencilStateCreateInfo BuildDepthStencilStateInfo(const PipelineDesc& desc);

public:
    Pipeline(const Device* device);
    ~Pipeline();

    bool Init(const PipelineDesc& desc);
};

} // namespace Renderer
} // namespace ABench
