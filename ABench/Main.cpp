#include "PCH.hpp"
#include "Common/Window.hpp"
#include "Common/Logger.hpp"
#include "Common/Timer.hpp"
#include "Common/FS.hpp"
#include "Renderer/HighLevel/Renderer.hpp"
#include "Math/Matrix.hpp"
#include "Scene/Camera.hpp"
#include "Scene/Mesh.hpp"
#include "Scene/Light.hpp"
#include "Scene/Scene.hpp"

#include "ResourceDir.hpp"


uint32_t windowWidth = 1280;
uint32_t windowHeight = 720;
ABench::Scene::Light* gLight;
ABench::Scene::Object* gLightObj;

class ABenchWindow: public ABench::Common::Window
{
    ABench::Scene::Camera mCamera;
    float mAngleX = 0.0f;
    float mAngleY = 0.0f;
    bool mLightFollowsCamera = false;

    void OnOpen() override
    {
        ABench::Scene::CameraDesc desc;
        desc.view.pos = ABench::Math::Vector4(0.0f, 0.0f, 2.0f, 1.0f);
        desc.view.at = ABench::Math::Vector4(0.0f, 0.0f, 1.0f, 1.0f);
        desc.view.up = ABench::Math::Vector4(0.0f,-1.0f, 0.0f, 0.0f); // to comply with Vulkan's coord system
        desc.fov = 60.0f;
        desc.aspect = static_cast<float>(GetWidth()) / static_cast<float>(GetHeight());
        desc.nearZ = 0.1f;
        desc.farZ = 2000.0f;
        mCamera.Init(desc);
    }

    void OnUpdate(float deltaTime) override
    {
        ABench::Math::Vector4 newPos;

        ABench::Math::Vector4 cameraFrontDir = mCamera.GetAtPosition() - mCamera.GetPosition();
        cameraFrontDir.Normalize();
        ABench::Math::Vector4 cameraRightDir = cameraFrontDir.Cross(mCamera.GetUpVector());
        ABench::Math::Vector4 cameraUpDir = cameraRightDir.Cross(cameraFrontDir);

        if (IsKeyPressed(ABench::Common::KeyCode::W)) newPos += cameraFrontDir;
        if (IsKeyPressed(ABench::Common::KeyCode::S)) newPos -= cameraFrontDir;
        if (IsKeyPressed(ABench::Common::KeyCode::D)) newPos += cameraRightDir;
        if (IsKeyPressed(ABench::Common::KeyCode::A)) newPos -= cameraRightDir;
        if (IsKeyPressed(ABench::Common::KeyCode::R)) newPos -= cameraUpDir;
        if (IsKeyPressed(ABench::Common::KeyCode::F)) newPos += cameraUpDir;

        // new direction
        ABench::Math::Vector4 updateDir;
        updateDir = ABench::Math::CreateRotationMatrixX(mAngleY) * ABench::Math::Vector4(0.0f, 0.0f, 1.0f, 0.0f);
        updateDir = ABench::Math::CreateRotationMatrixY(mAngleX) * updateDir;
        updateDir.Normalize();

        float speed = 5.0f;

        // TODO it would be better to update the position in a more friendly way
        ABench::Scene::CameraUpdateDesc desc;
        desc.pos = mCamera.GetPosition() + (newPos * speed * deltaTime);
        desc.at = desc.pos + updateDir;
        desc.up = mCamera.GetUpVector();
        mCamera.Update(desc);

        // Light
        ABench::Math::Vector4 lightNewPos;
        float lightSpeed = 2.0f;

        if (mLightFollowsCamera)
        {
            lightNewPos = mCamera.GetPosition();
            gLightObj->SetPosition(lightNewPos);
        }
        else
        {
            if (IsKeyPressed(ABench::Common::KeyCode::I)) lightNewPos.Data()[0] += lightSpeed;
            if (IsKeyPressed(ABench::Common::KeyCode::K)) lightNewPos.Data()[0] -= lightSpeed;
            if (IsKeyPressed(ABench::Common::KeyCode::L)) lightNewPos.Data()[2] += lightSpeed;
            if (IsKeyPressed(ABench::Common::KeyCode::J)) lightNewPos.Data()[2] -= lightSpeed;
            if (IsKeyPressed(ABench::Common::KeyCode::U)) lightNewPos.Data()[1] += lightSpeed;
            if (IsKeyPressed(ABench::Common::KeyCode::O)) lightNewPos.Data()[1] -= lightSpeed;
            gLightObj->SetPosition(gLightObj->GetPosition() + (lightNewPos * deltaTime));
        }
    }

    void OnMouseMove(int x, int y, int deltaX, int deltaY) override
    {
        UNUSED(x);
        UNUSED(y);

        if (IsMouseKeyPressed(0))
        {
            mAngleX += deltaX * 0.005f;
            mAngleY += deltaY * 0.005f;
        }
    }

    void OnKeyDown(ABench::Common::KeyCode key)
    {
        if (key == ABench::Common::KeyCode::T)
            mLightFollowsCamera ^= true;
    }

public:
    ABENCH_INLINE const ABench::Scene::Camera& GetCamera() const
    {
        return mCamera;
    }
};

int main()
{
    std::string path = ABench::Common::FS::GetParentDir(ABench::Common::FS::GetExecutablePath());
    if (!ABench::Common::FS::SetCWD(path + "/../../.."))
        return -1;

    if (!ABench::Common::FS::Exists(ABench::ResourceDir::SHADER_CACHE))
        ABench::Common::FS::CreateDir(ABench::ResourceDir::SHADER_CACHE);

    ABenchWindow window;
    window.Init();
    if (!window.Open(200, 200, windowWidth, windowHeight, "ABench"))
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

    ABench::Scene::Scene scene;
    scene.Init(ABench::Common::FS::JoinPaths(ABench::ResourceDir::SCENES, "sponza.fbx"));

    auto matResult = scene.GetMaterial("boxMaterial");
    ABench::Scene::Material* boxMat = matResult.first;
    if (matResult.second)
    {
        ABench::Scene::MaterialDesc matDesc;
        matDesc.diffusePath = ABench::Common::FS::JoinPaths(ABench::ResourceDir::TEXTURES, "Wood_Box_Diffuse.jpg");
        if (!boxMat->Init(matDesc))
        {
            LOGE("Failed to initialize material");
            return -1;
        }
    }

    matResult = scene.GetMaterial("boxMaterial2");
    ABench::Scene::Material* boxMatNoTex = matResult.first;
    if (matResult.second)
    {
        ABench::Scene::MaterialDesc matDesc;
        matDesc.color = ABench::Math::Vector4(0.2f, 0.4f, 0.9f, 1.0f);
        if (!boxMatNoTex->Init(matDesc))
        {
            LOGE("Failed to initialize no tex material");
            return -1;
        }
    }

    ABench::Scene::ModelDesc modelDesc;
    modelDesc.materials.push_back(boxMat);

    // textured cube
    auto modelResult = scene.GetComponent(ABench::Scene::ComponentType::Model, "box1");
    ABench::Scene::Model* model1 = dynamic_cast<ABench::Scene::Model*>(modelResult.first);
    if (modelResult.second)
        model1->Init(modelDesc);

    modelDesc.materials.clear();
    modelDesc.materials.push_back(boxMatNoTex);

    // untextured cube
    modelResult = scene.GetComponent(ABench::Scene::ComponentType::Model, "box2");
    ABench::Scene::Model* model2 = dynamic_cast<ABench::Scene::Model*>(modelResult.first);
    if (modelResult.second)
        model2->Init(modelDesc);

    ABench::Scene::Object* obj = scene.CreateObject();
    obj->SetComponent(model1);
    obj->SetPosition(0.0f, 1.0f, 0.0f);

    obj = scene.CreateObject();
    obj->SetComponent(model2);
    obj->SetPosition(2.0f, 1.0f, 0.0f);

    auto lightResult = scene.GetComponent(ABench::Scene::ComponentType::Light, "light");
    gLight = dynamic_cast<ABench::Scene::Light*>(lightResult.first);
    gLight->SetDiffuseIntensity(ABench::Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f));

    gLightObj = scene.CreateObject();
    gLightObj->SetComponent(gLight);
    gLightObj->SetPosition(3.0f, 3.0f, 0.0f);

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

#if defined(_DEBUG) && defined(WIN32)
    system("PAUSE");
#endif

    return 0;
}
