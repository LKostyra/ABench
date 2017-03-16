#pragma once

#include <vector>


namespace ABench {
namespace Renderer {

class Texture
{
    friend class Backbuffer;
    friend class RenderPass;
    friend class Framebuffer;

    uint32_t mWidth;
    uint32_t mHeight;
    VkFormat mFormat;
    std::vector<VkImage> mImages;
    std::vector<VkImageView> mImageViews;
    uint32_t mCurrentBuffer;

public:
    Texture();
    ~Texture();

    __forceinline uint32_t GetWidth() const
    {
        return mWidth;
    }

    __forceinline uint32_t GetHeight() const
    {
        return mHeight;
    }
};

} // namespace Renderer
} // namespace ABench