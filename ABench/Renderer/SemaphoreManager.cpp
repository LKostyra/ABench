#include "../PCH.hpp"
#include "SemaphoreManager.hpp"
#include "Extensions.hpp"
#include "Util.hpp"
#include "Device.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

SemaphoreManager::SemaphoreManager(Device* device)
    : mDevice(device)
    , mRenderSemaphore(VK_NULL_HANDLE)
    , mPresentSemaphore(VK_NULL_HANDLE)
    , mPostPresentSemaphore(VK_NULL_HANDLE)
{
}

SemaphoreManager::~SemaphoreManager()
{
    if (mPostPresentSemaphore)
        vkDestroySemaphore(mDevice->mDevice, mPostPresentSemaphore, nullptr);
    if (mPresentSemaphore)
        vkDestroySemaphore(mDevice->mDevice, mPresentSemaphore, nullptr);
    if (mRenderSemaphore)
        vkDestroySemaphore(mDevice->mDevice, mRenderSemaphore, nullptr);
}

bool SemaphoreManager::Init()
{
    VkSemaphoreCreateInfo semInfo;
    ZERO_MEMORY(semInfo);
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkResult result = vkCreateSemaphore(mDevice->mDevice, &semInfo, nullptr, &mRenderSemaphore);
    CHECK_VKRESULT(result, "Failed to create Render Semaphore");

    result = vkCreateSemaphore(mDevice->mDevice, &semInfo, nullptr, &mPresentSemaphore);
    CHECK_VKRESULT(result, "Failed to create Present Semaphore");

    result = vkCreateSemaphore(mDevice->mDevice, &semInfo, nullptr, &mPostPresentSemaphore);
    CHECK_VKRESULT(result, "Failed to create Post Present Semaphore");

    return true;
}

VkSemaphore SemaphoreManager::GetRenderSemaphore() const
{
     return mRenderSemaphore;
}

VkSemaphore SemaphoreManager::GetPresentSemaphore() const
{
     return mPresentSemaphore;
}

VkSemaphore SemaphoreManager::GetPostPresentSemaphore() const
{
    return mPostPresentSemaphore;
}

} // namespace Renderer
} // namespace ABench
