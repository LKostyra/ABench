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
    , mPrePresentSemaphore(VK_NULL_HANDLE)
    , mPresentSemaphore(VK_NULL_HANDLE)
    , mPostPresentSemaphore(VK_NULL_HANDLE)
{
}

SemaphoreManager::~SemaphoreManager()
{
    if (mPostPresentSemaphore)
        vkDestroySemaphore(mDevice->GetDevice(), mPostPresentSemaphore, nullptr);
    if (mPresentSemaphore)
        vkDestroySemaphore(mDevice->GetDevice(), mPresentSemaphore, nullptr);
    if (mPrePresentSemaphore)
        vkDestroySemaphore(mDevice->GetDevice(), mPrePresentSemaphore, nullptr);
    if (mRenderSemaphore)
        vkDestroySemaphore(mDevice->GetDevice(), mRenderSemaphore, nullptr);
}

bool SemaphoreManager::Init()
{
    VkSemaphoreCreateInfo semInfo;
    ZERO_MEMORY(semInfo);
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkResult result = vkCreateSemaphore(mDevice->GetDevice(), &semInfo, nullptr, &mRenderSemaphore);
    RETURN_FALSE_IF_FAILED(result, "Failed to create Render Semaphore");

    result = vkCreateSemaphore(mDevice->GetDevice(), &semInfo, nullptr, &mPrePresentSemaphore);
    RETURN_FALSE_IF_FAILED(result, "Failed to create Pre Present Semaphore");

    result = vkCreateSemaphore(mDevice->GetDevice(), &semInfo, nullptr, &mPresentSemaphore);
    RETURN_FALSE_IF_FAILED(result, "Failed to create Present Semaphore");

    result = vkCreateSemaphore(mDevice->GetDevice(), &semInfo, nullptr, &mPostPresentSemaphore);
    RETURN_FALSE_IF_FAILED(result, "Failed to create Post Present Semaphore");

    return true;
}

VkSemaphore SemaphoreManager::GetRenderSemaphore() const
{
     return mRenderSemaphore;
}

VkSemaphore SemaphoreManager::GetPrePresentSemaphore() const
{
    return mPrePresentSemaphore;
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
