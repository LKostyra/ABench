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
    VkImageLayout layout;
    uint32_t mipmapCount;
    TextureDataDesc* data;

    TextureDesc()
        : format(VK_FORMAT_UNDEFINED)
        , width(0)
        , height(0)
        , usage(0)
        , layout(VK_IMAGE_LAYOUT_UNDEFINED)
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
    VkImageLayout mImageLayout;
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

    ABENCH_INLINE VkImageView GetView() const
    {
        return mImageView;
    }

    ABENCH_INLINE VkImageLayout GetImageLayout() const
    {
        return mImageLayout;
    }

    ABENCH_INLINE const VkDescriptorSet GetDescriptorSet() const
    {
        return mImageDescriptorSet;
    }

    void Transition(CommandBuffer* cmdBuffer, VkPipelineStageFlags fromStage, VkPipelineStageFlags toStage,
                    VkAccessFlags fromAccess, VkAccessFlags toAccess,
                    uint32_t fromQueueFamily, uint32_t toQueueFamily,
                    VkImageLayout targetLayout);
    bool AllocateDescriptorSet(VkDescriptorSetLayout layout);
};

} // namespace Renderer
} // namespace ABench