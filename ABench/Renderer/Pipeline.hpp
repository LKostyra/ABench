#pragma once

#include "Device.hpp"
#include "Shader.hpp"
#include "VertexLayout.hpp"


namespace ABench {
namespace Renderer {

// TODO create Prerequisites.hpp
class Shader;

struct PipelineDesc
{
    Device* devicePtr;

    Shader* vertexShader;
    Shader* tessControlShader;
    Shader* tessEvalShader;
    Shader* geometryShader;
    Shader* fragmentShader;

    VertexLayout* vertexLayout;
    VkPrimitiveTopology topology;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;

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
    {
    }
};

class Pipeline
{
    friend class CommandBuffer;

    Device* mDevicePtr;

    VkPipeline mPipeline;

    void BuildShaderStageInfo(const PipelineDesc& desc, std::vector<VkPipelineShaderStageCreateInfo>& stages);
    VkPipelineVertexInputStateCreateInfo BuildVertexInputStateInfo(const PipelineDesc& desc);
    VkPipelineInputAssemblyStateCreateInfo BuildInputAssemblyStateInfo(const PipelineDesc& desc);
    VkPipelineTessellationStateCreateInfo BuildTessellationStateInfo();
    VkPipelineViewportStateCreateInfo BuildViewportStateInfo();
    VkPipelineRasterizationStateCreateInfo BuildRasterizationStateInfo();
    VkPipelineMultisampleStateCreateInfo BuildMultisampleStateInfo();
    VkPipelineDepthStencilStateCreateInfo BuildDepthStencilStateInfo();

public:
    Pipeline();
    ~Pipeline();

    bool Init(const PipelineDesc& desc);
};

} // namespace Renderer
} // namespace ABench
