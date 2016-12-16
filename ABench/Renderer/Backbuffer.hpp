#pragma once

#include "Instance.hpp"
#include "Device.hpp"

namespace ABench {
namespace Renderer {

struct BackbufferDesc
{
    HINSTANCE hInstance;
    HWND hWnd;
    Device* device;

    BackbufferDesc()
        : hInstance(0)
        , hWnd(0)
        , device(nullptr)
    {
    }
};

class Backbuffer
{
    VkSurfaceKHR mSurface;
    uint32_t mPresentQueueIndex;

public:
    Backbuffer();
    ~Backbuffer();

    bool Init(const Instance& inst, const BackbufferDesc& desc);
};

} // namespace Renderer
} // namespace ABench
