#include "PCH.hpp"
#include "WinBackbuffer.hpp"
#include "../Backbuffer.hpp"

#include "Common/Common.hpp"
#include "../Util.hpp"
#include "../Extensions.hpp"

namespace ABench {
namespace Renderer {

bool Backbuffer::CreateSurface(const BackbufferDesc& desc)
{
    VkWin32SurfaceCreateInfoKHR surfInfo;
    ZERO_MEMORY(surfInfo);
    surfInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfInfo.hinstance = desc.windowDesc.hInstance;
    surfInfo.hwnd = desc.windowDesc.hWnd;
    VkResult result = vkCreateWin32SurfaceKHR(mInstance->GetVkInstance(), &surfInfo, nullptr, &mSurface);
    RETURN_FALSE_IF_FAILED(result, "Failed to create Vulkan Surface for Win32");

    return true;
}

} // namespace ABench
} // namespace Renderer