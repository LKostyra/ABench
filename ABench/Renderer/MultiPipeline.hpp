#pragma once

#include "Pipeline.hpp"

namespace ABench {
namespace Renderer {

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

struct MultiPipelineShaderMacros
{
    ShaderMacros vertexShader;
    ShaderMacros tessControlShader;
    ShaderMacros tessEvalShader;
    ShaderMacros geometryShader;
    ShaderMacros fragmentShader;
};

struct MultiPipelineDesc
{
    MultiPipelineShaderDesc vertexShader;
    MultiPipelineShaderDesc tessControlShader;
    MultiPipelineShaderDesc tessEvalShader;
    MultiPipelineShaderDesc geometryShader;
    MultiPipelineShaderDesc fragmentShader;

    PipelineDesc pipelineDesc; // shaders from this desc are ignored
};

class MultiPipeline
{
    // TODO cache Pipelines
    Pipeline mBasePipeline;
    PipelineDesc mBasePipelineDesc;

    PipelineMap mPipelines;

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

    uint32_t CalculateAllCombinations(const MultiPipelineShaderMacroLimits& macros);
    void AdvanceCombinations(ShaderMacros& comb, const MultiPipelineShaderMacroLimits& macros);
    ShaderPtr GenerateShader(const std::string& path, const ShaderMacros& comb, ShaderType type);
    bool GenerateShaderModules(const MultiPipelineShaderDesc& desc, ShaderType type, ShaderMap* targetMap);

    PipelinePtr GenerateNewPipeline(const MultiPipelineShaderMacros& comb);

public:
    MultiPipeline();
    ~MultiPipeline();

    bool Init(const MultiPipelineDesc& desc);
    VkPipeline GetPipelineWithShaders(const MultiPipelineShaderMacros& combs);
};

} // namespace Renderer
} // namespace ABench
