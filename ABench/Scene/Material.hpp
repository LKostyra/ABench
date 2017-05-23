#pragma once

#include "Common/Image.hpp"
#include "Renderer/Texture.hpp"


namespace ABench {
namespace Scene {

struct MaterialDesc
{
    Common::Image* diffuse;
    // TODO place for normal map, specular map and else
};

class Material
{
    friend class Mesh;

    Renderer::Texture mDiffuse;

    bool CreateRendererTexture(Common::Image* image, VkImageUsageFlags usage);

public:
    Material();
    ~Material();

    bool Init(const MaterialDesc& desc);

    ABENCH_INLINE const VkImageView GetImageView() const
    {
        return mDiffuse.GetVkImageView(0);
    }
};

} // namespace Scene
} // namespace ABench
