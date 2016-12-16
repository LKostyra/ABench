#include "PCH.hpp"
#include <iostream>
#include "Common/Window.hpp"
#include "Renderer/Instance.hpp"
#include "Renderer/Device.hpp"
#include "Renderer/Backbuffer.hpp"

ABench::Common::Window gWindow;

int main()
{
    gWindow.Init();
    if (!gWindow.Open(300, 300, 200, 200, "ABench"))
    {
        return -1;
    }

    ABench::Renderer::Instance inst;
    if (!inst.Init())
    {
        return -1;
    }

    ABench::Renderer::Device dev;
    dev.Init(inst);

    ABench::Renderer::Backbuffer bb;
    ABench::Renderer::BackbufferDesc bbDesc;
    bbDesc.hInstance = gWindow.GetInstance();
    bbDesc.hWnd = gWindow.GetHandle();
    bbDesc.device = &dev;
    bb.Init(inst, bbDesc);

    while(gWindow.IsOpen())
    {
        gWindow.ProcessMessages();
    }

    return 0;
}
