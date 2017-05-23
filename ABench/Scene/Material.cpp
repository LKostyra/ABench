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

    switch (image->GetColorType())
    {
    case FIC_RGBALPHA:
        texDesc.format = VK_FORMAT_B8G8R8A8_UNORM;
        break;
    default:
        LOGE("Incorrect color type in loaded image - cannot match format");
        return false;
    }

    texDesc.dataSize = 4 * texDesc.width * texDesc.height;

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
