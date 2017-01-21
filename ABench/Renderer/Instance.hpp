#pragma once

#include <Common/Library.hpp>

namespace ABench {
namespace Renderer {

// An object representing Vulkan's VkInstance
class Instance
{
    friend class Device;

    VkInstance mInstance;
    Common::Library mVulkanLibrary;
    bool mDebuggingEnabled;

public:
    Instance();
    ~Instance();

    bool Init(VkDebugReportFlagsEXT debugFlags = 0);
    const VkInstance& GetVkInstance() const;
    bool IsDebuggingEnabled() const;
};

} // namespace Renderer
} // namespace ABench