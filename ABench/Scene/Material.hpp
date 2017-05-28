#pragma once

#include "Common/Image.hpp"
#include "Renderer/Texture.hpp"


namespace ABench {
namespace Scene {

struct MaterialDesc
{
    std::string diffusePath;
    // TODO place for normal map, specular map and else
};

class Material
{
    friend class Mesh;

    Common::Image mDiffuseImage;
    Renderer::Texture mDiffuseTexture;

    bool CreateRendererTexture(const std::string& image, VkImageUsageFlags usage);

public:
    Material();
    ~Material();

    bool Init(const MaterialDesc& desc);

    ABENCH_INLINE const VkDescriptorSet GetDescriptor() const
    {
        return mDiffuseTexture.GetDescriptorSet();
    }
};

} // namespace Scene
} // namespace ABench
