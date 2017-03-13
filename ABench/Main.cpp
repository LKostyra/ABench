#include "PCH.hpp"
#include <iostream>
#include "Common/Window.hpp"
#include "Renderer/Instance.hpp"
#include "Renderer/Device.hpp"
#include "Renderer/Backbuffer.hpp"
#include "Renderer/RenderPass.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/VertexLayout.hpp"
#include "Renderer/Tools.hpp"

ABench::Common::Window gWindow;

uint32_t windowWidth = 800;
uint32_t windowHeight = 600;

int main()
{
    gWindow.Init();
    if (!gWindow.Open(300, 300, windowWidth, windowHeight, "ABench"))
        return -1;

    VkDebugReportFlagsEXT debugFlags = 0;

#ifdef _DEBUG
    debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
#endif

    ABench::Renderer::Instance inst;
    if (!inst.Init(debugFlags))
        return -1;

    ABench::Renderer::Device dev;
    if (!dev.Init(inst))
        return -1;

    ABench::Renderer::Tools tools(&dev);

    ABench::Renderer::Backbuffer bb(&inst, &dev);
    ABench::Renderer::BackbufferDesc bbDesc;
    bbDesc.hInstance = gWindow.GetInstance();
    bbDesc.hWnd = gWindow.GetHandle();
    bbDesc.requestedFormat = VK_FORMAT_B8G8R8A8_UNORM;
    bbDesc.width = windowWidth;
    bbDesc.height = windowHeight;
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
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // 1
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // 2
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, // 3

        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // 1
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, // 3
        -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // 4
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

    ABench::Renderer::VertexLayout vertexLayout;
    ABench::Renderer::VertexLayoutDesc vlDesc;

    std::vector<ABench::Renderer::VertexLayoutEntry> vlEntries;
    vlEntries.push_back({VK_FORMAT_R32G32B32_SFLOAT, 0, 0, 28, false});
    vlEntries.push_back({VK_FORMAT_R32G32B32A32_SFLOAT, 12, 0, 28, false});

    vlDesc.entryCount = static_cast<uint32_t>(vlEntries.size());
    vlDesc.entries = vlEntries.data();
    if (!vertexLayout.Init(vlDesc))
        return -1;

    ABench::Renderer::Shader vertShader(&dev);
    ABench::Renderer::ShaderDesc shaderDesc;
    shaderDesc.language = ABench::Renderer::ShaderLanguage::SPIRV;
    shaderDesc.path = "Data/Shaders/vert.spv";
    if (!vertShader.Init(shaderDesc))
        return -1;

    ABench::Renderer::Shader fragShader(&dev);
    shaderDesc.path = "Data/Shaders/frag.spv";
    if (!fragShader.Init(shaderDesc))
        return -1;

    VkPipelineLayout pipeLayout = tools.CreatePipelineLayout(nullptr, 0);
    if (pipeLayout == VK_NULL_HANDLE)
        return -1;

    ABench::Renderer::Pipeline pipeline(&dev);
    ABench::Renderer::PipelineDesc pipeDesc;
    pipeDesc.vertexShader = &vertShader;
    pipeDesc.fragmentShader = &fragShader;
    pipeDesc.vertexLayout = &vertexLayout;
    pipeDesc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipeDesc.renderPass = &rp;
    pipeDesc.pipelineLayout = pipeLayout;
    pipeline.Init(pipeDesc);

    while(gWindow.IsOpen())
    {
        gWindow.ProcessMessages();

        {
            cmdBuf.Begin();

            cmdBuf.SetViewport(0, 0, windowWidth, windowHeight, 0.0f, 1.0f);
            cmdBuf.SetScissor(0, 0, windowWidth, windowHeight);

            float clearValue[] = {0.2f, 0.4f, 0.8f, 0.0f};
            cmdBuf.BeginRenderPass(&rp, &fb, clearValue);

            cmdBuf.BindPipeline(&pipeline);
            cmdBuf.BindVertexBuffer(&vertexBuffer);
            cmdBuf.Draw(6);

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
