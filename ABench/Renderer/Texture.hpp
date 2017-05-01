#pragma once

#include "Device.hpp"


namespace ABench {
namespace Renderer {

struct TextureDesc
{
    Device* devicePtr;

    VkFormat format;
    uint32_t width;
    uint32_t height;
    VkImageUsageFlags usage;

    TextureDesc()
        : devicePtr(nullptr)
        , format(VK_FORMAT_UNDEFINED)
        , width(0)
        , height(0)
        , usage(0)
    {
    }
};

struct ImageData
{
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
    VkImageLayout currentLayout;

    ImageData()
        : image(VK_NULL_HANDLE)
        , view(VK_NULL_HANDLE)
        , memory(VK_NULL_HANDLE)
        , currentLayout(VK_IMAGE_LAYOUT_UNDEFINED)
    {
    }
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
    bool mFromSwapchain;

    uint32_t mCurrentBuffer;

    VkImageLayout mDefaultLayout;
    VkImageSubresourceRange mSubresourceRange;

public:
    Texture();
    ~Texture();

    bool Init(const TextureDesc& desc);

    ABENCH_INLINE uint32_t GetWidth() const
    {
        return mWidth;
    }

    ABENCH_INLINE uint32_t GetHeight() const
    {
        return mHeight;
    }

    ABENCH_INLINE uint32_t GetCurrentBuffer() const
    {
        return mCurrentBuffer;
    }

    // Records a layout transition in given command buffer. UNDEFINED layout reverts to default.
    void Transition(VkCommandBuffer cmdBuffer, VkImageLayout targetLayout = VK_IMAGE_LAYOUT_UNDEFINED);
};

} // namespace Renderer
} // namespace ABench