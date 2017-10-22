#pragma once

#include "Common/Image.hpp"
#include "Renderer/Texture.hpp"


namespace ABench {
namespace Scene {

struct MaterialDesc
{
    std::string materialName;
    std::string diffusePath;
    std::string normalPath;
    std::string maskPath;
    // TODO place for any other needed textures
};

class Material
{
    friend class Mesh;

    std::string mMaterialName;

    Renderer::Texture mDiffuseTexture;
    Renderer::Texture mNormalTexture;
    Renderer::Texture mMaskTexture;

    bool CreateRendererTexture(const std::string& image, VkImageUsageFlags usage, Renderer::Texture& texture);

public:
    Material(const std::string& name);
    ~Material();

    bool Init(const MaterialDesc& desc);

    ABENCH_INLINE const VkDescriptorSet GetDiffuseDescriptor() const
    {
        return mDiffuseTexture.GetDescriptorSet();
    }

    ABENCH_INLINE const VkDescriptorSet GetNormalDescriptor() const
    {
        return mNormalTexture.GetDescriptorSet();
    }

    ABENCH_INLINE const VkDescriptorSet GetMaskDescriptor() const
    {
        return mMaskTexture.GetDescriptorSet();
    }
};

} // namespace Scene
} // namespace ABench
