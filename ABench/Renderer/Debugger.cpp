#include "../PCH.hpp"
#include "Debugger.hpp"
#include "Util.hpp"
#include "Extensions.hpp"

#include "Common/Common.hpp"
#include "Common/Logger.hpp"

namespace ABench {
namespace Renderer {

namespace {

VkBool32 DebugReport(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                     uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix,
                     const char* pMessage, void* pUserData)
{
    UNUSED(objectType);
    UNUSED(object);
    UNUSED(location);
    UNUSED(pUserData); // for now, but it might be a good idea to use it in the future

    const char* report = "VK_REPORT";
    const char* perf = "VK_PERF";

    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        LOGE(report << "(code " << messageCode << ", layer " << pLayerPrefix << "): " << pMessage);
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        LOGW(report << "(code " << messageCode << ", layer " << pLayerPrefix << "): " << pMessage);
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        LOGI(report << "(code " << messageCode << ", layer " << pLayerPrefix << "): " << pMessage);
    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        LOGD(report << "(code " << messageCode << ", layer " << pLayerPrefix << "): " << pMessage);
    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        LOGW(perf << "(code " << messageCode << ", layer " << pLayerPrefix << "): " << pMessage);

    // returning VK_TRUE here causes Vulkan APIs to return VK_ERROR_VALIDATION_FAILED_EXT
   /* if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        return VK_TRUE;
    else*/
        return VK_FALSE;
}

} // namespace


Debugger::Debugger()
    : mDebugCallback(VK_NULL_HANDLE)
    , mVkInstance(VK_NULL_HANDLE)
{
}

Debugger::~Debugger()
{
    Release();
}

Debugger& Debugger::Instance()
{
    static Debugger instance;
    return instance;
}

bool Debugger::InitReport(VkInstance instance, VkDebugReportFlagsEXT flags)
{
    mVkInstance = instance;

    vkCreateDebugReportCallbackEXT =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(mVkInstance, "vkCreateDebugReportCallbackEXT");
    vkDestroyDebugReportCallbackEXT =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(mVkInstance, "vkDestroyDebugReportCallbackEXT");
    vkDebugReportMessageEXT =
        (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(mVkInstance, "vkDebugReportMessageEXT");

    if (!vkCreateDebugReportCallbackEXT || !vkDestroyDebugReportCallbackEXT || !vkDebugReportMessageEXT)
    {
        LOGE("Failed to initialize Vulkan debugging layer - extension unavailable");
        return false;
    }

    VkDebugReportCallbackCreateInfoEXT debugInfo;
    ZERO_MEMORY(debugInfo);
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debugInfo.pfnCallback = reinterpret_cast<PFN_vkDebugReportCallbackEXT>(DebugReport);
    debugInfo.flags = flags;
    VkResult result = vkCreateDebugReportCallbackEXT(mVkInstance, &debugInfo, nullptr, &mDebugCallback);
    CHECK_VKRESULT(result, "Failed to allocate debug report callback");

    LOGD("Vulkan debug report layer initialized successfully.");
    return true;
}

void Debugger::Release()
{
    if (mDebugCallback)
    {
        vkDestroyDebugReportCallbackEXT(mVkInstance, mDebugCallback, nullptr);
        mDebugCallback = VK_NULL_HANDLE;
    }
}

} // namespace Renderer
} // namespace ABench
