#pragma once

#include "Common/Image.hpp"
#include "Renderer/Texture.hpp"
#include "Math/Vector.hpp"


namespace ABench {
namespace Scene {

struct MaterialDesc
{
    std::string materialName;

    Math::Vector color;
    std::string diffusePath;
    std::string normalPath;
    std::string maskPath;

    MaterialDesc()
        : materialName()
        , color(1.0f)
        , diffusePath()
        , normalPath()
        , maskPath()
    {
    }
};

class Material
{
    friend class Mesh;

    std::string mMaterialName;

    Math::Vector mColor;
    Renderer::Texture mDiffuseTexture;
    Renderer::Texture mNormalTexture;
    Renderer::Texture mMaskTexture;

    bool CreateRendererTexture(const std::string& image, VkImageUsageFlags usage, Renderer::Texture& texture);

public:
    Material(const std::string& name);
    ~Material();

    bool Init(const MaterialDesc& desc);

    ABENCH_INLINE const Math::Vector& GetColor() const
    {
        return mColor;
    }

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
