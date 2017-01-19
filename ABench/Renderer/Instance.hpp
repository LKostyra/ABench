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

public:
    Instance();
    ~Instance();

    bool Init();
    const VkInstance& GetVkInstance() const;
};

} // namespace Renderer
} // namespace ABench