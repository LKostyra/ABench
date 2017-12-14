#include "PCH.hpp"
#include "DescriptorLayoutManager.hpp"

#include "Renderer/LowLevel/Util.hpp"
#include "Renderer/LowLevel/Extensions.hpp"


namespace ABench {
namespace Renderer {

DescriptorLayoutManager::DescriptorLayoutManager()
    : mDevice()
    , mFragmentShaderSampler()
    , mVertexShaderLayout()
    , mFragmentShaderLayout()
    , mFragmentShaderDiffuseTextureLayout()
    , mFragmentShaderNormalTextureLayout()
    , mFragmentShaderMaskTextureLayout()
    , mAllShaderLayout()
{
}

DescriptorLayoutManager::~DescriptorLayoutManager()
{
}

DescriptorLayoutManager& DescriptorLayoutManager::Instance()
{
    static DescriptorLayoutManager instance;
    return instance;
}

bool DescriptorLayoutManager::Init(const DevicePtr& device)
{
    mDevice = device;

    // sampler for layouts below
    mFragmentShaderSampler = Tools::CreateSampler(mDevice);
    if (!mFragmentShaderSampler)
        return false;


    // shader resources initialization
    std::vector<DescriptorSetLayoutDesc> vsLayoutDesc;
    vsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, VK_NULL_HANDLE});
    vsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_NULL_HANDLE});
    mVertexShaderLayout = Tools::CreateDescriptorSetLayout(mDevice, vsLayoutDesc);
    if (!mVertexShaderLayout)
        return false;

    std::vector<DescriptorSetLayoutDesc> fsLayoutDesc;
    fsLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE});
    mFragmentShaderLayout = Tools::CreateDescriptorSetLayout(mDevice, fsLayoutDesc);
    if (!mFragmentShaderLayout)
        return false;

    std::vector<DescriptorSetLayoutDesc> fsTexLayoutDesc;
    fsTexLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, mFragmentShaderSampler});
    mFragmentShaderDiffuseTextureLayout = Tools::CreateDescriptorSetLayout(mDevice, fsTexLayoutDesc);
    if (!mFragmentShaderDiffuseTextureLayout)
        return false;

    mFragmentShaderNormalTextureLayout = Tools::CreateDescriptorSetLayout(mDevice, fsTexLayoutDesc);
    if (!mFragmentShaderNormalTextureLayout)
        return false;

    mFragmentShaderMaskTextureLayout = Tools::CreateDescriptorSetLayout(mDevice, fsTexLayoutDesc);
    if (!mFragmentShaderMaskTextureLayout)
        return false;

    std::vector<DescriptorSetLayoutDesc> allLayoutDesc;
    allLayoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, VK_NULL_HANDLE});
    mAllShaderLayout = Tools::CreateDescriptorSetLayout(mDevice, allLayoutDesc);
    if (!mAllShaderLayout)
        return false;

    return true;
}

} // namespace Renderer
} // namespace ABench
