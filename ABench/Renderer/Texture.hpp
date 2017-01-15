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
};

} // namespace Renderer
} // namespace ABench