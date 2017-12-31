#pragma once

#include "Prerequisites.hpp"
#include "Device.hpp"
#include "Common/Common.hpp"


namespace ABench {
namespace Renderer {

struct TextureDataDesc
{
    void* data;
    uint32_t dataSize;

    TextureDataDesc()
        : data(nullptr)
        , dataSize(0)
    {
    }

    TextureDataDesc(void* data, uint32_t dataSize)
        : data(data)
        , dataSize(dataSize)
    {
    }
};

struct TextureDesc
{
    VkFormat format;
    uint32_t width;
    uint32_t height;
    VkImageUsageFlags usage;
    uint32_t mipmapCount;
    TextureDataDesc* data;

    TextureDesc()
        : format(VK_FORMAT_UNDEFINED)
        , width(0)
        , height(0)
        , usage(0)
        , mipmapCount(1)
        , data(nullptr)
    {
    }
};

class Texture
{
    friend class RenderPass;
    friend class Framebuffer;
    friend class CommandBuffer;

    DevicePtr mDevice;

    uint32_t mWidth;
    uint32_t mHeight;
    VkFormat mFormat;
    VkImage mImage;
    VkImageView mImageView;
    VkDeviceMemory mImageMemory;
    VkImageLayout mCurrentLayout;
    VkImageLayout mDefaultLayout;
    VkImageSubresourceRange mSubresourceRange;
    VkDescriptorSet mImageDescriptorSet;

public:
    Texture();
    ~Texture();

    bool Init(const DevicePtr& device, const TextureDesc& desc);

    ABENCH_INLINE uint32_t GetWidth() const
    {
        return mWidth;
    }

    ABENCH_INLINE uint32_t GetHeight() const
    {
        return mHeight;
    }

    ABENCH_INLINE VkFormat GetFormat() const
    {
        return mFormat;
    }

    ABENCH_INLINE VkImage GetImage() const
    {
        return mImage;
    }

    ABENCH_INLINE VkImageLayout GetCurrentLayout() const
    {
        return mCurrentLayout;
    }

    ABENCH_INLINE const VkDescriptorSet GetDescriptorSet() const
    {
        return mImageDescriptorSet;
    }

    // Records a layout transition in given command buffer. UNDEFINED layout reverts to default.
    void Transition(VkCommandBuffer cmdBuffer, VkImageLayout targetLayout = VK_IMAGE_LAYOUT_UNDEFINED);
};

using TexturePtr = std::shared_ptr<Texture>;

} // namespace Renderer
} // namespace ABench