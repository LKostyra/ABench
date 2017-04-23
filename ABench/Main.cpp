#include "PCH.hpp"
#include <iostream>
#include "Common/Window.hpp"
#include "Common/Timer.hpp"
#include "Renderer/Renderer.hpp"
#include "Scene/Camera.hpp"


uint32_t windowWidth = 1280;
uint32_t windowHeight = 720;

class ABenchWindow: public ABench::Common::Window
{
    ABench::Scene::Camera mCamera;

    void OnOpen() override
    {
        ABench::Scene::CameraDesc desc;
        desc.view.pos = ABench::Math::Vector(0.0f, 0.0f, 2.0f, 1.0f);
        desc.view.at = ABench::Math::Vector(0.0f, 0.0f, 0.0f, 1.0f);
        desc.view.up = ABench::Math::Vector(0.0f, 1.0f, 0.0f, 0.0f);
        desc.fov = 60.0f;
        desc.aspect = static_cast<float>(GetWidth()) / static_cast<float>(GetHeight());
        desc.nearZ = 0.1f;
        desc.farZ = 1000.0f;
        mCamera.Init(desc);
    }

    void OnUpdate(float deltaTime) override
    {
        ABench::Math::Vector updateDir;

        ABench::Math::Vector cameraFrontDir = mCamera.GetAtPosition() - mCamera.GetPosition();
        cameraFrontDir.Normalize();
        ABench::Math::Vector cameraRightDir = cameraFrontDir.Cross(mCamera.GetUpVector());
        ABench::Math::Vector cameraUpDir = cameraRightDir.Cross(cameraFrontDir);

        if (mKeys['W']) updateDir += cameraFrontDir;
        if (mKeys['S']) updateDir -= cameraFrontDir;
        if (mKeys['D']) updateDir += cameraRightDir;
        if (mKeys['A']) updateDir -= cameraRightDir;
        if (mKeys['R']) updateDir += cameraUpDir;
        if (mKeys['F']) updateDir -= cameraUpDir;

        // TODO it would be better to update the position in a more friendly way
        ABench::Scene::CameraUpdateDesc desc;
        desc.pos = mCamera.GetPosition() + (updateDir * 3.0f * deltaTime);
        desc.at = mCamera.GetAtPosition();
        desc.up = mCamera.GetUpVector();
        mCamera.Update(desc);
    }

public:
    __forceinline const ABench::Scene::Camera& GetCamera() const
    {
        return mCamera;
    }
};

int main()
{
    ABenchWindow window;

    window.Init();
    if (!window.Open(300, 300, windowWidth, windowHeight, "ABench"))
        return -1;

    bool debug = false;

#ifdef _DEBUG
    debug = true;
#endif

    ABench::Renderer::Renderer rend;
    if (!rend.Init(window, debug, false))
        return -1;

    std::vector<std::unique_ptr<ABench::Scene::Mesh>> meshes;

    for (int x = -5; x < 5; ++x)
    {
        for (int y = -5; y < 5; ++y)
        {
            meshes.emplace_back(new ABench::Scene::Mesh());

            meshes.back()->Init(rend.GetDevice(), "");
            meshes.back()->SetPosition(x*1.5f, y*1.5f, 0.0f);
            rend.AddMesh(meshes.back().get());
        }
    }

    ABench::Common::Timer timer;
    timer.Start();

    while(window.IsOpen())
    {
        float frameTime = static_cast<float>(timer.Stop());
        timer.Start();
        float fps = 1.0f / frameTime;

        window.SetTitle("ABench - " + std::to_string(fps) + " FPS (" + std::to_string(frameTime * 1000.0f) + " ms)"); 
        window.Update(frameTime);
        rend.Draw(window.GetCamera());
    }

    system("PAUSE");

    return 0;
}
