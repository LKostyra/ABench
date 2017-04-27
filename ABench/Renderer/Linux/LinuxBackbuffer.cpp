#include "../../PCH.hpp"
#include "LinuxBackbuffer.hpp"
#include "../Backbuffer.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

bool Backbuffer::CreateSurface(const BackbufferDesc& desc)
{
    UNUSED(desc);
    return false;
}

} // namespace ABench
} // namespace Renderer