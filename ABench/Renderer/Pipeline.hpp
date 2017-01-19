#pragma once

#include "Device.hpp"
#include "Shader.hpp"
#include "VertexLayout.hpp"
#include "RenderPass.hpp"


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
    RenderPass* renderPass;
};

class Pipeline
{
    const Device* mDevicePtr;

    VkPipeline mPipeline;

    std::vector<VkPipelineShaderStageCreateInfo> BuildShaderStageInfo(const PipelineDesc& desc);
    VkPipelineVertexInputStateCreateInfo BuildVertexInputStateInfo(const PipelineDesc& desc);
    VkPipelineInputAssemblyStateCreateInfo BuildInputAssemblyStateInfo(const PipelineDesc& desc);

public:
    Pipeline(const Device* device);
    ~Pipeline();

    bool Init(const PipelineDesc& desc);
};

} // namespace Renderer
} // namespace ABench
