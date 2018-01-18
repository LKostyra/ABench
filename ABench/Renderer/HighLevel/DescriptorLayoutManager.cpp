#include "PCH.hpp"
#include "DescriptorLayoutManager.hpp"

#include "Renderer/LowLevel/Util.hpp"
#include "Renderer/LowLevel/Extensions.hpp"


namespace ABench {
namespace Renderer {

DescriptorLayoutManager::DescriptorLayoutManager()
    : mDevice()
    , mFragmentShaderSampler()
    , mFragmentShaderDiffuseTextureLayout()
    , mFragmentShaderNormalTextureLayout()
    , mFragmentShaderMaskTextureLayout()
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

    return true;
}

} // namespace Renderer
} // namespace ABench
