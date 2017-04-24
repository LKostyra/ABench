#include "PCH.hpp"
#include "Instance.hpp"

#include "Util.hpp"
#include "Extensions.hpp"
#include "Debugger.hpp"

#include "Common/Common.hpp"
#include "Common/Logger.hpp"


namespace ABench {
namespace Renderer {

Instance::Instance()
    : mInstance(VK_NULL_HANDLE)
    , mVulkanLibrary()
    , mDebuggingEnabled(false)
{
}

Instance::~Instance()
{
    Debugger::Instance().Release();

    if (mInstance)
        vkDestroyInstance(mInstance, nullptr);
}

bool Instance::Init(VkDebugReportFlagsEXT debugFlags)
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

    std::vector<const char*> enabledExtensions;
    enabledExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    if (debugFlags)
        enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

#ifdef WIN32
    enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__linux__) | defined(__LINUX__)
    enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#else
    #error "Target platform not supported."
#endif

    const char* enabledLayers[] = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    VkInstanceCreateInfo instInfo;
    ZERO_MEMORY(instInfo);
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pNext = nullptr;
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    instInfo.ppEnabledExtensionNames = enabledExtensions.data();
    instInfo.enabledLayerCount = 1;
    instInfo.ppEnabledLayerNames = enabledLayers;

    VkResult result = vkCreateInstance(&instInfo, nullptr, &mInstance);
    RETURN_FALSE_IF_FAILED(result, "Failed to create Vulkan Instance");

    if (!InitInstanceExtensions(mInstance))
    {
        LOGE("Failed to initialize all Instance extensions");
        return false;
    }

    if (debugFlags)
    {
        if (!Debugger::Instance().InitReport(mInstance, debugFlags))
            LOGW("Failed to initialize Debug Reports - debugging unavailable");

        mDebuggingEnabled = true;
    }

    LOGI("Vulkan Instance initialized successfully");
    return true;
}

const VkInstance& Instance::GetVkInstance() const
{
    return mInstance;
}

bool Instance::IsDebuggingEnabled() const
{
    return mDebuggingEnabled;
}

} // namespace Renderer
} // namespace ABench
