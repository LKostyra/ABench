#pragma once

#include "Instance.hpp"
#include "Device.hpp"
#include "Texture.hpp"

namespace ABench {
namespace Renderer {

struct BackbufferDesc
{
    HINSTANCE hInstance;
    HWND hWnd;
    Instance* instance;
    Device* device;
    VkFormat requestedFormat;
    bool vsync;
    uint32_t bufferCount;
    uint32_t width;
    uint32_t height;

    BackbufferDesc()
        : hInstance(0)
        , hWnd(0)
        , instance(nullptr)
        , device(nullptr)
        , requestedFormat(VK_FORMAT_UNDEFINED)
        , vsync(false)
        , bufferCount(2)
        , width(800)
        , height(600)
    {
    }
};

class Backbuffer: public Texture
{
    VkSurfaceKHR mSurface;
    uint32_t mPresentQueueIndex;
    VkQueue mPresentQueue;
    VkColorSpaceKHR mColorSpace;
    VkPresentModeKHR mPresentMode;
    VkSurfaceCapabilitiesKHR mSurfCaps;
    uint32_t mBufferCount;
    VkSwapchainKHR mSwapchain;

    std::vector<VkCommandBuffer> mPresentCommandBuffers;

    bool CreateSurface(const BackbufferDesc& desc);
    bool GetPresentQueue(const BackbufferDesc& desc);
    bool SelectSurfaceFormat(const BackbufferDesc& desc);
    bool SelectPresentMode(const BackbufferDesc& desc);
    bool AcquireSurfaceCaps(const BackbufferDesc& desc);
    void SelectBufferCount(const BackbufferDesc& desc);
    bool CreateSwapchain(const BackbufferDesc& desc);
    bool AllocateImageViews(const BackbufferDesc& desc);

public:
    Backbuffer();
    ~Backbuffer();

    bool Init(const BackbufferDesc& desc);
};

} // namespace Renderer
} // namespace ABench
