#include "PCH.hpp"
#include "Material.hpp"
#include "Common/Logger.hpp"


namespace ABench {
namespace Scene {

Material::Material(const std::string& name)
    : mMaterialName(name)
{
}

Material::~Material()
{
}

bool Material::CreateRendererTexture(const std::string& image, VkImageUsageFlags usage)
{
    if (!mDiffuseImage.Init(image))
        return false;

    Renderer::TextureDesc texDesc;
    texDesc.width = mDiffuseImage.GetWidth();
    texDesc.height = mDiffuseImage.GetHeight();
    texDesc.usage = usage;
    texDesc.data = mDiffuseImage.GetData();

    switch (mDiffuseImage.GetColorType())
    {
    case FIC_RGBALPHA:
        texDesc.format = VK_FORMAT_B8G8R8A8_UNORM;
        break;
    default:
        LOGE("Incorrect color type in loaded image - cannot match format");
        return false;
    }

    texDesc.dataSize = 4 * texDesc.width * texDesc.height;

    return mDiffuseTexture.Init(texDesc);
}

bool Material::Init(const MaterialDesc& desc)
{

    if (desc.diffusePath.empty())
    {
        LOGE("Diffuse texture is required");
        return false;
    }

    return CreateRendererTexture(desc.diffusePath, VK_IMAGE_USAGE_SAMPLED_BIT);
}

} // namespace Scene
} // namespace ABench
