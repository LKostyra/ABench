#pragma once

#include "Instance.hpp"
#include "Device.hpp"
#include "Texture.hpp"

namespace ABench {
namespace Renderer {

struct BackbufferDesc
{
    Instance* instancePtr;
    Device* devicePtr;

    HINSTANCE hInstance;
    HWND hWnd;
    VkFormat requestedFormat;
    bool vsync;
    uint32_t bufferCount;
    uint32_t width;
    uint32_t height;

    BackbufferDesc()
        : hInstance(0)
        , hWnd(0)
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
    Instance* mInstancePtr;

    VkSurfaceKHR mSurface;
    uint32_t mPresentQueueIndex;
    VkQueue mPresentQueue;
    VkColorSpaceKHR mColorSpace;
    VkPresentModeKHR mPresentMode;
    VkSurfaceCapabilitiesKHR mSurfCaps;
    uint32_t mBufferCount;
    VkSwapchainKHR mSwapchain;
    VkFence mImageAcquireFence;

    bool CreateSurface(const BackbufferDesc& desc);
    bool GetPresentQueue(const BackbufferDesc& desc);
    bool SelectSurfaceFormat(const BackbufferDesc& desc);
    bool SelectPresentMode(const BackbufferDesc& desc);
    bool AcquireSurfaceCaps(const BackbufferDesc& desc);
    void SelectBufferCount(const BackbufferDesc& desc);
    bool CreateSwapchain(const BackbufferDesc& desc);
    bool AllocateImageViews(const BackbufferDesc& desc);
    bool CreateImageAcquireFences();

public:
    Backbuffer();
    ~Backbuffer();

    bool Init(const BackbufferDesc& desc);

    // Present current image on screen. This should be usually called at the end of current frame.
    bool Present();

    // Acquire new image. This should be called at the beginning of the frame.
    bool AcquireNextImage();
};

} // namespace Renderer
} // namespace ABench
