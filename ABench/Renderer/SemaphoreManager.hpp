#pragma once


namespace ABench {
namespace Renderer {

class Device;

class SemaphoreManager
{
    friend class Device;
    friend class Backbuffer;

    const Device* mDevice;

    VkSemaphore mRenderSemaphore;
    VkSemaphore mPresentSemaphore;
    VkSemaphore mPostPresentSemaphore;

public:
    SemaphoreManager(Device* device);
    ~SemaphoreManager();

    bool Init();

    VkSemaphore GetRenderSemaphore() const;
    VkSemaphore GetPresentSemaphore() const;
    VkSemaphore GetPostPresentSemaphore() const;
};

} // namespace Renderer
} // namespace ABench
