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
    if (!mDiffuseImage.Init(image, true))
        return false;

    std::vector<Renderer::TextureDataDesc> textures;
    textures.reserve(mDiffuseImage.GetMipmapCount());
    for (uint32_t i = 0; i < mDiffuseImage.GetMipmapCount(); ++i)
        textures.emplace_back(mDiffuseImage.GetSubimageData(i), mDiffuseImage.GetSubimageSize(i));

    Renderer::TextureDesc texDesc;
    texDesc.width = mDiffuseImage.GetWidth();
    texDesc.height = mDiffuseImage.GetHeight();
    texDesc.usage = usage;
    texDesc.data = textures.data();
    texDesc.mipmapCount = static_cast<uint32_t>(textures.size());

    switch (mDiffuseImage.GetColorType())
    {
    case FIC_RGBALPHA:
        texDesc.format = VK_FORMAT_B8G8R8A8_UNORM;
        break;
    default:
        LOGE("Incorrect color type in loaded image - cannot match format");
        return false;
    }

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
