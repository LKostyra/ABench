#include "PCH.hpp"
#include "Material.hpp"
#include "Common/Logger.hpp"


namespace ABench {
namespace Scene {

Material::Material()
{
}

Material::~Material()
{
}

bool Material::CreateRendererTexture(Common::Image* image, VkImageUsageFlags usage)
{
    Renderer::TextureDesc texDesc;
    texDesc.width = image->GetWidth();
    texDesc.height = image->GetHeight();
    texDesc.usage = usage;
    texDesc.data = image->GetData();

    uint32_t bpp = 0;
    switch (image->GetColorType())
    {
    case FIC_RGB:
        texDesc.format = VK_FORMAT_R8G8B8_UINT;
        bpp = 24;
        break;
    case FIC_RGBALPHA:
        texDesc.format = VK_FORMAT_R8G8B8A8_UINT;
        bpp = 32;
        break;
    default:
        LOGE("Incorrect color type in loaded image - cannot match format");
        return false;
    }

    texDesc.dataSize = bpp * texDesc.width * texDesc.height;

    return mDiffuse.Init(texDesc);
}

bool Material::Init(const MaterialDesc& desc)
{
    if (!desc.diffuse)
    {
        LOGE("Diffuse texture is required");
        return false;
    }

    return CreateRendererTexture(desc.diffuse, VK_IMAGE_USAGE_SAMPLED_BIT);
}

} // namespace Scene
} // namespace ABench
