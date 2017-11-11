#pragma once

namespace ABench {
namespace Renderer {

enum DeviceQueueType
{
    GRAPHICS = 0,
    TRANSFER,
    COMPUTE,
    COUNT // used only as a counter
};

enum ClearType
{
    ABENCH_CLEAR_NONE = 0,
    ABENCH_CLEAR_COLOR,
    ABENCH_CLEAR_DEPTH,
    ABENCH_CLEAR_ALL = ABENCH_CLEAR_COLOR | ABENCH_CLEAR_DEPTH,
};

} // namespace Renderer
} // namespace ABench
