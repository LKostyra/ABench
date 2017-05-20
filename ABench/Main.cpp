#include "PCH.hpp"
#include <iostream>
#include "Common/Window.hpp"
#include "Common/Logger.hpp"
#include "Common/Timer.hpp"
#include "Renderer/Renderer.hpp"
#include "Math/Matrix.hpp"
#include "Scene/Camera.hpp"
#include "Scene/Mesh.hpp"
#include "Scene/Scene.hpp"


uint32_t windowWidth = 1280;
uint32_t windowHeight = 720;

class ABenchWindow: public ABench::Common::Window
{
    ABench::Scene::Camera mCamera;
    float mAngleX = 0.0f;
    float mAngleY = 0.0f;

    void OnOpen() override
    {
        ABench::Scene::CameraDesc desc;
        desc.view.pos = ABench::Math::Vector(0.0f, 0.0f, 2.0f, 1.0f);
        desc.view.at = ABench::Math::Vector(0.0f, 0.0f, 1.0f, 1.0f);
        desc.view.up = ABench::Math::Vector(0.0f, 1.0f, 0.0f, 0.0f);
        desc.fov = 60.0f;
        desc.aspect = static_cast<float>(GetWidth()) / static_cast<float>(GetHeight());
        desc.nearZ = 0.1f;
        desc.farZ = 2000.0f;
        mCamera.Init(desc);
    }

    void OnUpdate(float deltaTime) override
    {
        ABench::Math::Vector newPos;

        ABench::Math::Vector cameraFrontDir = mCamera.GetAtPosition() - mCamera.GetPosition();
        cameraFrontDir.Normalize();
        ABench::Math::Vector cameraRightDir = cameraFrontDir.Cross(mCamera.GetUpVector());
        ABench::Math::Vector cameraUpDir = cameraRightDir.Cross(cameraFrontDir);

        if (IsKeyPressed(ABench::Common::KeyCode::W)) newPos += cameraFrontDir;
        if (IsKeyPressed(ABench::Common::KeyCode::S)) newPos -= cameraFrontDir;
        if (IsKeyPressed(ABench::Common::KeyCode::D)) newPos += cameraRightDir;
        if (IsKeyPressed(ABench::Common::KeyCode::A)) newPos -= cameraRightDir;
        if (IsKeyPressed(ABench::Common::KeyCode::R)) newPos += cameraUpDir;
        if (IsKeyPressed(ABench::Common::KeyCode::F)) newPos -= cameraUpDir;

        // new direction
        ABench::Math::Vector updateDir;
        updateDir = ABench::Math::CreateRotationMatrixX(mAngleY) * ABench::Math::Vector(0.0f, 0.0f, 1.0f, 0.0f);
        updateDir = ABench::Math::CreateRotationMatrixY(mAngleX) * updateDir;
        updateDir.Normalize();

        float speed = 5.0f;

        // TODO it would be better to update the position in a more friendly way
        ABench::Scene::CameraUpdateDesc desc;
        desc.pos = mCamera.GetPosition() + (newPos * speed * deltaTime);
        desc.at = desc.pos + updateDir;
        desc.up = mCamera.GetUpVector();
        mCamera.Update(desc);
    }

    void OnMouseMove(int x, int y, int deltaX, int deltaY) override
    {
        UNUSED(x);
        UNUSED(y);

        if (IsMouseKeyPressed(0))
        {
            mAngleX += -deltaX * 0.005f;
            mAngleY += -deltaY * 0.005f;
        }
    }

public:
    ABENCH_INLINE const ABench::Scene::Camera& GetCamera() const
    {
        return mCamera;
    }
};

int main()
{
    ABenchWindow window;

    window.Init();
    if (!window.Open(300, 300, windowWidth, windowHeight, "ABench"))
    {
        LOGE("Failed to initialize Window");
        return -1;
    }

    bool debug = false;

#ifdef _DEBUG
    debug = true;
#endif

    ABench::Renderer::Renderer rend;
    if (!rend.Init(window, debug, false))
    {
        LOGE("Failed to initialize Renderer");
        return -1;
    }

    ABench::Scene::Mesh mesh;
    mesh.Init();

    ABench::Scene::Scene scene;
    scene.Init("Data/FBX/sponza.fbx");

    ABench::Common::Timer timer;
    timer.Start();

    while(window.IsOpen())
    {
        float frameTime = static_cast<float>(timer.Stop());
        timer.Start();
        float fps = 1.0f / frameTime;

        window.SetTitle("ABench - " + std::to_string(fps) + " FPS (" + std::to_string(frameTime * 1000.0f) + " ms)");
        window.Update(frameTime);
        rend.Draw(scene, window.GetCamera());
    }

#ifdef _DEBUG
    system("PAUSE");
#endif

    return 0;
}
