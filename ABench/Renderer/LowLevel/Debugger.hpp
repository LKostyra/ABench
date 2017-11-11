#pragma once

#include "Prerequisites.hpp"

#define DEBUG_ALL_FLAGS VK_DEBUG_REPORT_ERROR_BIT_EXT | \
                        VK_DEBUG_REPORT_WARNING_BIT_EXT | \
                        VK_DEBUG_REPORT_INFORMATION_BIT_EXT | \
                        VK_DEBUG_REPORT_DEBUG_BIT_EXT | \
                        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT

namespace ABench {
namespace Renderer {

class Debugger
{
    // some instance functions to debug
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
    PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;

    VkDebugReportCallbackEXT mDebugCallback;

    VkInstance mVkInstance;

    Debugger();
    Debugger(const Debugger&) = delete;
    Debugger(Debugger&&) = delete;
    Debugger& operator=(const Debugger&) = delete;
    Debugger& operator=(Debugger&&) = delete;
    ~Debugger();

public:
    static Debugger& Instance();

    bool InitReport(VkInstance instance, VkDebugReportFlagsEXT flags = DEBUG_ALL_FLAGS);
    void Release();
};

} // namespace Renderer
} // namespace ABench
