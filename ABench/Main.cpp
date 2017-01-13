#include "PCH.hpp"
#include <iostream>
#include "Common/Window.hpp"
#include "Renderer/Instance.hpp"
#include "Renderer/Device.hpp"
#include "Renderer/Backbuffer.hpp"
#include "Renderer/RenderPass.hpp"
#include "Renderer/Framebuffer.hpp"

ABench::Common::Window gWindow;

int main()
{
    gWindow.Init();
    if (!gWindow.Open(300, 300, 200, 200, "ABench"))
        return -1;

    ABench::Renderer::Instance inst;
    if (!inst.Init())
        return -1;

    ABench::Renderer::Device dev;
    if (!dev.Init(inst))
        return -1;

    ABench::Renderer::Backbuffer bb(&inst, &dev);
    ABench::Renderer::BackbufferDesc bbDesc;
    bbDesc.hInstance = gWindow.GetInstance();
    bbDesc.hWnd = gWindow.GetHandle();
    bbDesc.requestedFormat = VK_FORMAT_B8G8R8A8_UNORM;
    if (!bb.Init(bbDesc))
        return -1;

    ABench::Renderer::RenderPass rp(&dev);
    ABench::Renderer::RenderPassDesc rpDesc;
    rpDesc.colorFormat = bbDesc.requestedFormat;
    if (!rp.Init(rpDesc))
        return -1;

    ABench::Renderer::Framebuffer fb(&dev);
    ABench::Renderer::FramebufferDesc fbDesc;
    fbDesc.colorTex = &bb;
    fbDesc.renderPass = &rp;
    if (!fb.Init(fbDesc))
        return -1;

    while(gWindow.IsOpen())
    {
        gWindow.ProcessMessages();
    }

    return 0;
}
