#include "PCH.hpp"
#include "LinuxBackbuffer.hpp"
#include "../Backbuffer.hpp"
#include "../Extensions.hpp"
#include "../Util.hpp"

#include "Common/Common.hpp"
#include "Common/Logger.hpp"

namespace ABench {
namespace Renderer {

bool Backbuffer::CreateSurface(const BackbufferDesc& desc)
{
    VkXcbSurfaceCreateInfoKHR info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    info.connection = desc.windowDesc.connection;
    info.window = desc.windowDesc.window;
    VkResult result = vkCreateXcbSurfaceKHR(mInstance->GetVkInstance(), &info, nullptr, &mSurface);
    RETURN_FALSE_IF_FAILED(result, "Failed to create Vulkan Surface for XCB");

    return true;
}

} // namespace ABench
} // namespace Renderer
