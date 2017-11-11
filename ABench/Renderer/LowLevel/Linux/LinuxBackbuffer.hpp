#pragma once

namespace ABench {
namespace Renderer {

struct BackbufferWindowDesc
{
    xcb_connection_t* connection;
    xcb_window_t window;

    BackbufferWindowDesc()
        : connection(nullptr)
        , window(0)
    {
    }
};

} // namespace ABench
} // namespace Renderer
