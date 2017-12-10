#pragma once

#include "Common/Image.hpp"
#include "Math/Vector.hpp"

#include "Renderer/LowLevel/Texture.hpp"
#include "Renderer/HighLevel/ResourceManager.hpp"


namespace ABench {
namespace Scene {

struct MaterialDesc
{
    std::string materialName;

    Math::Vector4 color;
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

    Math::Vector4 mColor;
    Renderer::TexturePtr mDiffuseTexture;
    Renderer::TexturePtr mNormalTexture;
    Renderer::TexturePtr mMaskTexture;

    bool CreateRendererTexture(const std::string& image, VkImageUsageFlags usage, Renderer::TexturePtr& texture);

public:
    Material(const std::string& name);
    ~Material();

    bool Init(const MaterialDesc& desc);

    ABENCH_INLINE const Math::Vector4& GetColor() const
    {
        return mColor;
    }

    ABENCH_INLINE const VkDescriptorSet GetDiffuseDescriptor() const
    {
        if (mDiffuseTexture)
            return mDiffuseTexture->GetDescriptorSet();
        else
            return VK_NULL_HANDLE;
    }

    ABENCH_INLINE const VkDescriptorSet GetNormalDescriptor() const
    {
        if (mNormalTexture)
            return mNormalTexture->GetDescriptorSet();
        else
            return VK_NULL_HANDLE;
    }

    ABENCH_INLINE const VkDescriptorSet GetMaskDescriptor() const
    {
        if (mMaskTexture)
            return mMaskTexture->GetDescriptorSet();
        else
            return VK_NULL_HANDLE;
    }
};

} // namespace Scene
} // namespace ABench
