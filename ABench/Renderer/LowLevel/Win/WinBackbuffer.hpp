#pragma once

namespace ABench {
namespace Renderer {

struct BackbufferWindowDesc
{
    HINSTANCE hInstance;
    HWND hWnd;

    BackbufferWindowDesc()
        : hInstance(0)
        , hWnd(0)
    {
    }
};

} // namespace ABench
} // namespace Renderer
