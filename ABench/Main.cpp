#include "PCH.hpp"
#include <iostream>
#include "Common/Window.hpp"
#include "Renderer/Instance.hpp"
#include "Renderer/Device.hpp"
#include "Renderer/Backbuffer.hpp"
#include "Renderer/RenderPass.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/Buffer.hpp"

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
    bbDesc.width = 200;
    bbDesc.height = 200;
    bbDesc.vsync = true;
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

    float vertices[] =
    {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f
    };

    ABench::Renderer::Buffer vertexBuffer(&dev);
    ABench::Renderer::BufferDesc vbDesc;
    vbDesc.data = vertices;
    vbDesc.dataSize = sizeof(vertices);
    vbDesc.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (!vertexBuffer.Init(vbDesc))
        return -1;

    ABench::Renderer::CommandBuffer cmdBuf(&dev);
    if (!cmdBuf.Init())
        return -1;


    while(gWindow.IsOpen())
    {
        std::cout << "\n======\nBEGIN NEXT FRAME\n======\n\n";

        gWindow.ProcessMessages();

        {
            cmdBuf.Begin();

            float clearValue[] = {1.0f, 0.0f, 0.0f, 1.0f};
            cmdBuf.BeginRenderPass(&rp, &fb, clearValue);

            cmdBuf.EndRenderPass();

            if (!cmdBuf.End())
                return -1;
        }


        dev.Execute(&cmdBuf);

        bb.Present();

        dev.WaitForGPU();
    }

    system("PAUSE");

    return 0;
}
