#pragma once

#include "Prerequisites.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "Texture.hpp"

#ifdef WIN32
#include "Win/WinBackbuffer.hpp"
#elif defined(__linux__) | defined(__LINUX__)
#include "Linux/LinuxBackbuffer.hpp"
#else
#error "Target platform not supported."
#endif

namespace ABench {
namespace Renderer {

struct BackbufferDesc
{
    InstancePtr instancePtr;

    BackbufferWindowDesc windowDesc;

    VkFormat requestedFormat;
    bool vsync;
    uint32_t bufferCount;
    uint32_t width;
    uint32_t height;

    BackbufferDesc()
        : requestedFormat(VK_FORMAT_UNDEFINED)
        , vsync(false)
        , bufferCount(2)
        , width(800)
        , height(600)
    {
    }
};

class Backbuffer: public Texture
{
    InstancePtr mInstance;
    DevicePtr mDevice;

    VkSurfaceKHR mSurface;
    uint32_t mPresentQueueIndex;
    VkQueue mPresentQueue;
    VkColorSpaceKHR mColorSpace;
    VkPresentModeKHR mPresentMode;
    VkSurfaceCapabilitiesKHR mSurfCaps;
    uint32_t mBufferCount;
    VkSwapchainKHR mSwapchain;

    bool CreateSurface(const BackbufferDesc& desc);
    bool GetPresentQueue();
    bool SelectSurfaceFormat(const BackbufferDesc& desc);
    bool SelectPresentMode(const BackbufferDesc& desc);
    bool AcquireSurfaceCaps();
    void SelectBufferCount(const BackbufferDesc& desc);
    bool CreateSwapchain(const BackbufferDesc& desc);
    bool AllocateImageViews();

public:
    Backbuffer();
    ~Backbuffer();

    bool Init(const DevicePtr& device, const BackbufferDesc& desc);

    // Present current image on screen. This should be usually called at the end of current frame.
    bool Present(VkSemaphore waitSemaphore);

    // Acquire new image. This should be called at the beginning of the frame.
    bool AcquireNextImage(VkSemaphore semaphore);
};

} // namespace Renderer
} // namespace ABench
