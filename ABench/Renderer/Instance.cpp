#include "../PCH.hpp"
#include "Instance.hpp"

#include "Util.hpp"
#include "Extensions.hpp"

#include "Common/Common.hpp"
#include "Common/Logger.hpp"

namespace ABench {
namespace Renderer {

Instance::Instance()
    : mInstance(VK_NULL_HANDLE)
    , mVulkanLibrary()
{
}

Instance::~Instance()
{
    if (mInstance)
        vkDestroyInstance(mInstance, nullptr);
}

bool Instance::Init()
{
    if (mInstance)
        return true; // already initialized

    if (!mVulkanLibrary.Open("vulkan-1"))
    {
        LOGE("Failed to open Vulkan library");
        return false;
    }

    if (!InitLibraryExtensions(mVulkanLibrary))
    {
        LOGE("Failed to initialize functions from Vulkan Library");
        return false;
    }

    VkApplicationInfo appInfo;
    ZERO_MEMORY(appInfo);
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "ABench";
    appInfo.pEngineName = "ABench";
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.applicationVersion = 1;

    const char* enabledExtensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    };

    VkInstanceCreateInfo instInfo;
    ZERO_MEMORY(instInfo);
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pNext = nullptr;
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledExtensionCount = 2;
    instInfo.ppEnabledExtensionNames = enabledExtensions;

    VkResult result = vkCreateInstance(&instInfo, nullptr, &mInstance);
    CHECK_VKRESULT(result, "Failed to create Vulkan Instance");

    if (!InitInstanceExtensions(mInstance))
    {
        LOGE("Failed to initialize all Instance extensions");
        return false;
    }

    LOGI("Vulkan Instance initialized successfully");
    return true;
}

const VkInstance& Instance::GetVkInstance() const
{
    return mInstance;
}

} // namespace Renderer
} // namespace ABench
