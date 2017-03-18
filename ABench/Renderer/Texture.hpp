#pragma once

#include "Device.hpp"


namespace ABench {
namespace Renderer {

struct TextureDesc
{
    Device* devicePtr;
};

struct ImageData
{
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
    VkImageLayout currentLayout;
};

class Texture
{
    friend class Backbuffer;
    friend class RenderPass;
    friend class Framebuffer;

    Device* mDevicePtr;

    uint32_t mWidth;
    uint32_t mHeight;
    VkFormat mFormat;
    std::vector<ImageData> mImages;
    uint32_t mCurrentBuffer;

    VkImageLayout mDefaultLayout;
    VkImageSubresourceRange mSubresourceRange;

public:
    Texture();
    ~Texture();

    bool Init(const TextureDesc& desc);

    __forceinline uint32_t GetWidth() const
    {
        return mWidth;
    }

    __forceinline uint32_t GetHeight() const
    {
        return mHeight;
    }

    __forceinline uint32_t GetCurrentBuffer() const
    {
        return mCurrentBuffer;
    }

    // Records a layout transition in given command buffer. UNDEFINED layout reverts to default.
    void Transition(VkCommandBuffer cmdBuffer, VkImageLayout targetLayout = VK_IMAGE_LAYOUT_UNDEFINED);
};

} // namespace Renderer
} // namespace ABench