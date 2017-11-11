#pragma once

#include "Pipeline.hpp"

namespace ABench {
namespace Renderer {

template <typename PipelineDescType>
struct BasePipeline
{
    Pipeline pipeline;
    PipelineDescType desc;
};

struct MultiPipelineShaderMacroDesc
{
    std::string name;
    unsigned char maxValue;
};

using MultiPipelineShaderMacroLimits = std::vector<MultiPipelineShaderMacroDesc>;
using PipelineMap = std::map<ShaderMacros, PipelinePtr>;
using ShaderMap = std::map<ShaderMacros, ShaderPtr>;

struct MultiPipelineShaderDesc
{
    std::string path;
    MultiPipelineShaderMacroLimits macros;
};

struct MultiGraphicsPipelineShaderMacros
{
    ShaderMacros vertexShader;
    ShaderMacros tessControlShader;
    ShaderMacros tessEvalShader;
    ShaderMacros geometryShader;
    ShaderMacros fragmentShader;
};

struct MultiGraphicsPipelineDesc
{
    MultiPipelineShaderDesc vertexShader;
    MultiPipelineShaderDesc tessControlShader;
    MultiPipelineShaderDesc tessEvalShader;
    MultiPipelineShaderDesc geometryShader;
    MultiPipelineShaderDesc fragmentShader;

    GraphicsPipelineDesc pipelineDesc; // shaders from this desc are ignored
};

struct MultiComputePipelineDesc
{
    MultiPipelineShaderDesc computeShader;

    ComputePipelineDesc pipelineDesc; // shader from this desc is ignored
};

class MultiPipeline
{
    DevicePtr mDevice;

    // Graphics Pipeline related stuff
    BasePipeline<GraphicsPipelineDesc> mBaseGraphicsPipeline;
    PipelineMap mGraphicsPipelines;
    std::string mVertexShaderPath;
    std::string mTessControlShaderPath;
    std::string mTessEvalShaderPath;
    std::string mGeometryShaderPath;
    std::string mFragmentShaderPath;
    ShaderMap mVertexShaders;
    ShaderMap mTessControlShaders;
    ShaderMap mTessEvalShaders;
    ShaderMap mGeometryShaders;
    ShaderMap mFragmentShaders;

    // Compute Pipeline related stuff
    BasePipeline<ComputePipelineDesc> mBaseComputePipeline;
    PipelineMap mComputePipelines;
    std::string mComputeShaderPath;
    ShaderMap mComputeShaders;

    uint32_t CalculateAllCombinations(const MultiPipelineShaderMacroLimits& macros);
    void AdvanceCombinations(ShaderMacros& comb, const MultiPipelineShaderMacroLimits& macros);
    ShaderPtr GenerateShader(const std::string& path, const ShaderMacros& comb, ShaderType type);
    bool GenerateShaderModules(const MultiPipelineShaderDesc& desc, ShaderType type, ShaderMap* targetMap);

    PipelinePtr GenerateNewPipeline(const MultiGraphicsPipelineShaderMacros& comb); // for graphics
    PipelinePtr GenerateNewPipeline(const ShaderMacros& comb); // for compute

public:
    MultiPipeline();
    ~MultiPipeline();

    bool Init(const DevicePtr& device, const MultiGraphicsPipelineDesc& desc);
    bool Init(const DevicePtr& device, const MultiComputePipelineDesc& desc);
    VkPipeline GetGraphicsPipeline(const MultiGraphicsPipelineShaderMacros& combs);
    VkPipeline GetComputePipeline(const ShaderMacros& comb);
};

} // namespace Renderer
} // namespace ABench
