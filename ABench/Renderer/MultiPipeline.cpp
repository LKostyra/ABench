#include "PCH.hpp"
#include "MultiPipeline.hpp"

#include "Common/Logger.hpp"


namespace ABench {
namespace Renderer {

MultiPipeline::MultiPipeline()
{
}

MultiPipeline::~MultiPipeline()
{
}

uint32_t MultiPipeline::CalculateAllCombinations(const MultiPipelineShaderMacroLimits& macros)
{
    uint32_t result = 1;

    for (auto& m: macros)
        result *= m.maxValue + 1;

    return result;
}

void MultiPipeline::AdvanceCombinations(ShaderMacros& comb, const MultiPipelineShaderMacroLimits& macros)
{
    ASSERT(comb.size() == macros.size(), "Requested shader macro combination does not match declared macros");

    for (int32_t i = static_cast<int32_t>(comb.size()) - 1; i >= 0; --i)
    {
        comb[i].value++;

        if (comb[i].value > macros[i].maxValue)
            comb[i].value = 0;
        else
            break;
    }
}

ShaderPtr MultiPipeline::GenerateShader(const std::string& path, const ShaderMacros& macros, ShaderType type)
{
    ShaderDesc sDesc;
    sDesc.filename = path;
    sDesc.type = type;
    sDesc.macros = macros;

    ShaderPtr s = std::make_shared<Shader>();
    s->Init(sDesc);

    return s;
}

bool MultiPipeline::GenerateShaderModules(const MultiPipelineShaderDesc& desc, ShaderType type, ShaderMap* targetMap)
{
    ShaderMacros macros;
    for (auto& m: desc.macros)
        macros.emplace_back(m.name, 0);
    uint32_t combinations = CalculateAllCombinations(desc.macros);

    targetMap->clear();
    for (uint32_t c = 0; c < combinations; ++c)
    {
        // pre-create shaders now for further use
        targetMap->insert(std::make_pair(macros, GenerateShader(desc.path, macros, type)));
        AdvanceCombinations(macros, desc.macros);
    }

    return true;
}

PipelinePtr MultiPipeline::GenerateNewPipeline(const MultiPipelineShaderMacros& comb)
{
    PipelinePtr p = std::make_shared<Pipeline>();

    mBasePipelineDesc.vertexShader = mVertexShaders[comb.vertexShader].get();
    mBasePipelineDesc.tessControlShader = mTessControlShaders[comb.tessControlShader].get();
    mBasePipelineDesc.tessEvalShader = mTessEvalShaders[comb.tessEvalShader].get();
    mBasePipelineDesc.geometryShader = mGeometryShaders[comb.geometryShader].get();
    mBasePipelineDesc.fragmentShader = mFragmentShaders[comb.fragmentShader].get();
    p->Init(mBasePipelineDesc);

    return p;
}

bool MultiPipeline::Init(const MultiPipelineDesc& desc)
{
    mPipelines.clear();

    if (desc.vertexShader.path.empty() || desc.fragmentShader.path.empty())
    {
        LOGE("To construct a Pipeline at least Vertex and Fragment shaders are required.");
        return false;
    }

    if (!GenerateShaderModules(desc.vertexShader, ShaderType::VERTEX, &mVertexShaders))
    {
        LOGE("Failed to generate vertex shader modules for multipipeline");
        return false;
    }

    if (!desc.tessControlShader.path.empty())
    {
        if (!GenerateShaderModules(desc.tessControlShader, ShaderType::TESS_CONTROL, &mTessControlShaders))
        {
            LOGE("Failed to generate tessellation control shader modules for multipipeline");
            return false;
        }
    }

    if (!desc.tessEvalShader.path.empty())
    {
        if (!GenerateShaderModules(desc.tessEvalShader, ShaderType::TESS_EVAL, &mTessEvalShaders))
        {
            LOGE("Failed to generate tessellation evaluation shader modules for multipipeline");
            return false;
        }
    }

    if (!desc.geometryShader.path.empty())
    {
        if (!GenerateShaderModules(desc.geometryShader, ShaderType::GEOMETRY, &mGeometryShaders))
        {
            LOGE("Failed to generate geometry shader modules for multipipeline");
            return false;
        }
    }

    if (!GenerateShaderModules(desc.fragmentShader, ShaderType::FRAGMENT, &mFragmentShaders))
    {
        LOGE("Failed to generate fragment shader modules for multipipeline");
        return false;
    }

    ShaderMacros macros;
    mBasePipelineDesc = desc.pipelineDesc;
    
    if (!desc.vertexShader.macros.empty())
        for (auto& m: desc.vertexShader.macros)
            macros.emplace_back(m.name, 0);
    
    mBasePipelineDesc.vertexShader = mVertexShaders[macros].get();
    mBasePipelineDesc.tessControlShader = nullptr;
    mBasePipelineDesc.tessEvalShader = nullptr;
    mBasePipelineDesc.geometryShader = nullptr;
    mBasePipelineDesc.fragmentShader = nullptr;
    mBasePipelineDesc.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

    if (!mBasePipeline.Init(mBasePipelineDesc))
    {
        LOGE("Failed to create a base pipeline");
        return false;
    }

    mBasePipelineDesc.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
    mBasePipelineDesc.basePipeline = mBasePipeline.GetPipeline();

    return true;
}

VkPipeline MultiPipeline::GetPipelineWithShaders(const MultiPipelineShaderMacros& combs)
{
    // TODO this needs some rethinking. It might turn out that mixing shaders like this is a bad idea,
    // and will lead to conflicts (will it? we are using one set of shaders with MultiPipeline anyway...)
    ShaderMacros macros;
    macros.insert(macros.end(), combs.vertexShader.begin(), combs.vertexShader.end());
    macros.insert(macros.end(), combs.tessControlShader.begin(), combs.tessControlShader.end());
    macros.insert(macros.end(), combs.tessEvalShader.begin(), combs.tessEvalShader.end());
    macros.insert(macros.end(), combs.geometryShader.begin(), combs.geometryShader.end());
    macros.insert(macros.end(), combs.fragmentShader.begin(), combs.fragmentShader.end());

    auto pipeline = mPipelines.find(macros);
    if (pipeline == mPipelines.end())
    {
        PipelinePtr p = GenerateNewPipeline(combs);
        auto result = mPipelines.insert(std::make_pair(macros, p));
        if (!result.second)
        {
            LOGE("Failed to create new pipeline");
            return VK_NULL_HANDLE;
        }

        pipeline = result.first;
    }

    return pipeline->second->GetPipeline();
}

} // namespace Renderer
} // namespace ABench
