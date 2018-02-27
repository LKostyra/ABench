#pragma once

#include "Common/Image.hpp"
#include "Math/Vector.hpp"

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

    ABENCH_INLINE const Renderer::TexturePtr& GetDiffuse() const
    {
        return mDiffuseTexture;
    }

    ABENCH_INLINE const Renderer::TexturePtr& GetNormal() const
    {
        return mNormalTexture;
    }

    ABENCH_INLINE const Renderer::TexturePtr& GetMask() const
    {
        return mMaskTexture;
    }
};

} // namespace Scene
} // namespace ABench
