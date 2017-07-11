#include "PCH.hpp"
#include "Shader.hpp"
#include "Util.hpp"
#include "Extensions.hpp"
#include "Renderer.hpp"

#include "Common/Common.hpp"
#include "Common/FS.hpp"
#include "ResourceDir.hpp"

#include <fstream>

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>

namespace ABench {
namespace Renderer {

namespace {

// default limits for glslang shader resources
// taken from glslang standalone implementation
const TBuiltInResource DEFAULT_LIMITS = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }};

const int DEFAULT_VERSION = 110;
const std::string SHADER_HEADER_START = "#version 450\n\
#extension GL_ARB_separate_shader_objects: enable\n\
#extension GL_ARB_shading_language_420pack: enable\n";
const std::string DEFINE_STR = "#define ";
const std::string SHADER_HEADER_TAIL = "\0";

EShLanguage GetShaderLanguage(ShaderType type)
{
    switch (type)
    {
    case ShaderType::VERTEX:        return EShLangVertex;
    case ShaderType::TESS_CONTROL:  return EShLangTessControl;
    case ShaderType::TESS_EVAL:     return EShLangTessEvaluation;
    case ShaderType::GEOMETRY:      return EShLangGeometry;
    case ShaderType::FRAGMENT:      return EShLangFragment;
    case ShaderType::COMPUTE:       return EShLangCompute;
    default:                        return EShLangCount;
    }
}

} // namespace



Shader::Shader()
    : mShaderModule(VK_NULL_HANDLE)
{
}

Shader::~Shader()
{
    if (mShaderModule != VK_NULL_HANDLE)
        vkDestroyShaderModule(gDevice->GetDevice(), mShaderModule, nullptr);
}

bool Shader::CompileToFile(ShaderType type, const std::string& filename, std::vector<uint32_t>& code)
{
    EShLanguage lang = GetShaderLanguage(type);
    if (lang == EShLangCount)
    {
        LOGE("Incorrect shader type provided");
        return false;
    }

    std::string shaderFile = Common::FS::JoinPaths(ResourceDir::SHADERS, filename);
    std::string spvShaderFile = shaderFile + ".spv";

    std::ifstream glslSource(shaderFile, std::ifstream::in);
    if (!glslSource.good())
    {
        LOGE("Failed to open GLSL source file " << filename);
        return false;
    }

    std::string shaderHead = SHADER_HEADER_START;
    // TODO macros
    shaderHead += SHADER_HEADER_TAIL;

    std::string shaderCode((std::istreambuf_iterator<char>(glslSource)), std::istreambuf_iterator<char>());
    shaderCode += SHADER_HEADER_TAIL;

    const char* shaderCodeArray[] = { shaderHead.c_str(), shaderCode.c_str() };
    std::unique_ptr<glslang::TShader> shader = std::make_unique<glslang::TShader>(lang);
    if (!shader)
    {
        LOGE("Failed to allocate glslang's shader for compilation");
        return false;
    }

    shader->setStrings(shaderCodeArray, 2);
    shader->setEntryPoint("main");

    glslang::TShader::ForbidIncluder includer;
    EShMessages msg = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
    if (!shader->parse(&DEFAULT_LIMITS, DEFAULT_VERSION, false, msg))
    {
        LOGE("Failed to parse shader file " << filename << ":");
        std::stringstream log(shader->getInfoLog());
        for (std::string line; std::getline(log, line); )
        {
            LOGE(line);
        }
        return false;
    }

    std::unique_ptr<glslang::TProgram> program = std::make_unique<glslang::TProgram>();
    if (!program)
    {
        LOGE("Failed to allocate glslang's program for compilation");
        return false;
    }

    program->addShader(shader.get());
    if (!program->link(msg))
    {
        LOGE("Failed to link shader " << filename << ":");
        std::stringstream log(program->getInfoLog());
        for (std::string line; std::getline(log, line); )
        {
            LOGE(line);
        }
        return false;
    }

    glslang::TIntermediate* intermediate = program->getIntermediate(lang);
    if (!intermediate)
    {
        LOGE("Failed to extract shader's intermediate representation");
        return false;
    }

    std::string errors;
    spv::SpvBuildLogger spvLogger;
    glslang::GlslangToSpv(*intermediate, code, &spvLogger);

    std::string logs = spvLogger.getAllMessages();
    if (!logs.empty())
    {
        LOGW("GlslangToSpv returned some messages for shader " << filename << ":\n" << spvLogger.getAllMessages());
        return false;
    }

    // save to file
    std::ofstream spvFile(spvShaderFile, std::ofstream::out | std::ofstream::binary);
    if (!spvFile)
    {
        LOGE("Unable to open SPV shader file " << filename << ".spv for writing");
        return false;
    }

    spvFile.write(reinterpret_cast<const char*>(code.data()), code.size() * sizeof(uint32_t));

    return true;
}

bool Shader::CreateVkShaderModule(const std::vector<uint32_t>& code)
{
    VkShaderModuleCreateInfo shaderInfo;
    ZERO_MEMORY(shaderInfo);
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.pCode = code.data();
    shaderInfo.codeSize = code.size() * sizeof(uint32_t);
    VkResult result = vkCreateShaderModule(gDevice->GetDevice(), &shaderInfo, nullptr, &mShaderModule);
    RETURN_FALSE_IF_FAILED(result, "Failed to create shader module");

    return true;
}

bool Shader::Init(const ShaderDesc& desc)
{
    std::vector<uint32_t> code;

    std::string shaderPath(Common::FS::JoinPaths(ResourceDir::SHADERS, desc.filename));
    std::string spvShaderPath(shaderPath + ".spv");

    if (!Common::FS::Exists(spvShaderPath))
    {
        LOGW("Generating SPV version of shader file " << desc.filename);

        if (!CompileToFile(desc.type, desc.filename, code))
        {
            LOGE("Failed to compile shader to SPIRV");
            return false;
        }
    }
    else if (Common::FS::GetFileModificationTime(shaderPath) > Common::FS::GetFileModificationTime(spvShaderPath))
    {
        LOGW("SPV version of shader " << desc.filename << " out of date - refreshing");

        if (!Common::FS::RemoveFile(spvShaderPath))
        {
            LOGE("Failed to remove existing shader code");
            return false;
        }

        if (!CompileToFile(desc.type, desc.filename, code))
        {
            LOGE("Failed to compile shader to SPIRV");
            return false;
        }   
    }
    else
    {
        std::ifstream codeFile(spvShaderPath, std::ifstream::in | std::ifstream::binary);
        if (!codeFile.good())
        {
            LOGE("Failed to open Shader code file " << desc.filename);
            return false;
        }

        codeFile.seekg(0, std::ifstream::end);
        size_t codeSize = static_cast<size_t>(codeFile.tellg());
        codeFile.seekg(0, std::ifstream::beg);

        code.resize(codeSize / sizeof(uint32_t));
        codeFile.read(reinterpret_cast<char*>(code.data()), codeSize);
        if (!codeFile.good())
        {
            LOGE("Failure while reading the file");
            LOGE("bits: " << codeFile.eof() << codeFile.fail() << codeFile.bad());
        }
    }

    if (!CreateVkShaderModule(code))
    {
        LOGE("Failed to create Vulkan Shader Module for shader " << desc.filename);
        return false;
    }

    LOGI("Successfully created shader from file " << desc.filename);
    return true;
}

} // namespace Renderer
} // namespace ABench
