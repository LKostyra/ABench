#pragma once

#include "Common/Image.hpp"
#include "Renderer/Texture.hpp"


namespace ABench {
namespace Scene {

struct MaterialDesc
{
    std::string materialName;
    std::string diffusePath;
    // TODO place for any other needed textures
};

class Material
{
    friend class Mesh;

    std::string mMaterialName;

    Renderer::Texture mDiffuseTexture;

    bool CreateRendererTexture(const std::string& image, VkImageUsageFlags usage);

public:
    Material(const std::string& name);
    ~Material();

    bool Init(const MaterialDesc& desc);

    ABENCH_INLINE const VkDescriptorSet GetDescriptor() const
    {
        return mDiffuseTexture.GetDescriptorSet();
    }
};

} // namespace Scene
} // namespace ABench
