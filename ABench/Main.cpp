#include "PCH.hpp"
#include "Common/Window.hpp"
#include "Common/Logger.hpp"
#include "Common/Timer.hpp"
#include "Common/FS.hpp"
#include "Renderer/HighLevel/Renderer.hpp"
#include "Math/Common.hpp"
#include "Math/Matrix.hpp"
#include "Math/RingAverage.hpp"
#include "Scene/Camera.hpp"
#include "Scene/Mesh.hpp"
#include "Scene/Light.hpp"
#include "Scene/Scene.hpp"

#include "ResourceDir.hpp"


uint32_t windowWidth = 1280;
uint32_t windowHeight = 720;
ABench::Scene::Light* gLight;

const int32_t EMITTERS_LIMIT = 1;
ABench::Scene::Emitter* gEmitters[EMITTERS_LIMIT * 2 + 1];

const int32_t ANIM_LIGHTS_LIMIT = 100;
ABench::Scene::Light* gAnimatedLights[ANIM_LIGHTS_LIMIT * 2 + 1];

class ABenchWindow: public ABench::Common::Window
{
    ABench::Scene::Camera mCamera;
    float mAngleX = 0.0f;
    float mAngleY = 0.0f;
    bool mLightFollowsCamera = false;
    bool mCameraOnRails = false;

    float mLightAnimationMoment = 0.0f;

    void OnOpen() override
    {
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

        ABench::Scene::CameraDesc desc;
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
            gLight->SetPosition(lightNewPos);
        }
        else
        {
            if (IsKeyPressed(ABench::Common::KeyCode::I)) lightNewPos.Data()[0] += lightSpeed;
            if (IsKeyPressed(ABench::Common::KeyCode::K)) lightNewPos.Data()[0] -= lightSpeed;
            if (IsKeyPressed(ABench::Common::KeyCode::L)) lightNewPos.Data()[2] += lightSpeed;
            if (IsKeyPressed(ABench::Common::KeyCode::J)) lightNewPos.Data()[2] -= lightSpeed;
            if (IsKeyPressed(ABench::Common::KeyCode::U)) lightNewPos.Data()[1] += lightSpeed;
            if (IsKeyPressed(ABench::Common::KeyCode::O)) lightNewPos.Data()[1] -= lightSpeed;
            gLight->SetPosition(gLight->GetPosition() + (lightNewPos * deltaTime));
        }

        // Animated lights
        for (int32_t i = -ANIM_LIGHTS_LIMIT; i < ANIM_LIGHTS_LIMIT + 1; ++i)
        {
            float YPos = 2.0f + sinf(mLightAnimationMoment + (i * 0.5f));
            gAnimatedLights[i + ANIM_LIGHTS_LIMIT]->SetPosition(i * 2.5f, YPos, 0.0f);
        }

        mLightAnimationMoment += deltaTime * 2.0f;
        if (mLightAnimationMoment > 2 * MATH_PIF)
            mLightAnimationMoment -= 2 * MATH_PIF;
    }

    void OnMouseMove(int x, int y, int deltaX, int deltaY) override
    {
        UNUSED(x);
        UNUSED(y);

        if (IsMouseKeyPressed(0))
        {
            mAngleX -= deltaX * 0.005f;
            mAngleY -= deltaY * 0.005f;
        }
    }

    void OnKeyDown(ABench::Common::KeyCode key) override
    {
        if (key == ABench::Common::KeyCode::T)
            mLightFollowsCamera ^= true;

        if (key == ABench::Common::KeyCode::F1)
            mCameraOnRails ^= true;
    }

public:
    ABENCH_INLINE ABench::Scene::Camera& GetCamera()
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
    ABench::Renderer::RendererDesc rendDesc;
    rendDesc.debugEnable = debug;
    rendDesc.debugVerbose = false;
    rendDesc.window = &window;
    rendDesc.fov = 60.0f;
    rendDesc.nearZ = 0.2f;
    rendDesc.farZ = 500.0f;
    if (!rend.Init(rendDesc))
    {
        LOGE("Failed to initialize Renderer");
        return -1;
    }

    ABench::Scene::CameraDesc desc;
    desc.pos = ABench::Math::Vector4(0.0f, 1.0f,-2.0f, 1.0f);
    desc.at = ABench::Math::Vector4(0.0f, 1.0f, 1.0f, 1.0f);
    desc.up = ABench::Math::Vector4(0.0f,-1.0f, 0.0f, 0.0f); // to comply with Vulkan's coord system
    window.GetCamera().Update(desc);

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
    {
        model1->Init(modelDesc);
        model1->SetPosition(-2.0f, 1.0f, 0.0f);
    }

    modelDesc.materials.clear();
    modelDesc.materials.push_back(boxMatNoTex);

    // untextured cube
    modelResult = scene.GetComponent(ABench::Scene::ComponentType::Model, "box2");
    ABench::Scene::Model* model2 = dynamic_cast<ABench::Scene::Model*>(modelResult.first);
    if (modelResult.second)
    {
        model2->Init(modelDesc);
        model2->SetPosition(2.0f, 1.0f, 0.0f);
    }

    ABench::Scene::Object* obj = scene.CreateObject();
    obj->SetComponent(model1);

    obj = scene.CreateObject();
    obj->SetComponent(model2);

    auto lightResult = scene.GetComponent(ABench::Scene::ComponentType::Light, "light");
    gLight = dynamic_cast<ABench::Scene::Light*>(lightResult.first);
    gLight->SetDiffuseIntensity(ABench::Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f));
    gLight->SetPosition(3.0f, 5.0f, 0.0f);

    ABench::Scene::Object* lightObj = scene.CreateObject();
    lightObj->SetComponent(gLight);

    for (int i = -ANIM_LIGHTS_LIMIT; i < ANIM_LIGHTS_LIMIT + 1; ++i)
    {
        auto lres = scene.GetComponent(ABench::Scene::ComponentType::Light, "light" + std::to_string(i + ANIM_LIGHTS_LIMIT));
        gAnimatedLights[i + ANIM_LIGHTS_LIMIT] = dynamic_cast<ABench::Scene::Light*>(lres.first);

        float colorX = static_cast<float>(i + ANIM_LIGHTS_LIMIT) / ((static_cast<float>(ANIM_LIGHTS_LIMIT) * 2.0f) + 1.0f);
        gAnimatedLights[i + ANIM_LIGHTS_LIMIT]->SetDiffuseIntensity(ABench::Math::Vector4(colorX, 1.0f, 1.0f - colorX, 1.0f));
        gAnimatedLights[i + ANIM_LIGHTS_LIMIT]->SetPosition(i * 2.5f, 2.0f, 0.0f);

        ABench::Scene::Object* o = scene.CreateObject();
        o->SetComponent(gAnimatedLights[i + ANIM_LIGHTS_LIMIT]);
    }

    for (int i = -EMITTERS_LIMIT; i < EMITTERS_LIMIT + 1; ++i)
    {
        auto emitterResult = scene.GetComponent(ABench::Scene::ComponentType::Emitter, "emitter" + std::to_string(i + EMITTERS_LIMIT));
        gEmitters[i + EMITTERS_LIMIT] = dynamic_cast<ABench::Scene::Emitter*>(emitterResult.first);

        gEmitters[i + EMITTERS_LIMIT]->SetParticleLimit(1024);
        gEmitters[i + EMITTERS_LIMIT]->SetSpawnPeriod(1.0f / 200.0f);
        gEmitters[i + EMITTERS_LIMIT]->SetLifeTime(4.0f);
        gEmitters[i + EMITTERS_LIMIT]->SetPosition(ABench::Math::Vector4(i * 6.0f, 10.0f, -0.25f, 1.0f));

        ABench::Scene::Object* o = scene.CreateObject();
        o->SetComponent(gEmitters[i + EMITTERS_LIMIT]);
    }

    ABench::Common::Timer timer;
    ABench::Math::RingAverage<float, 30> avgTime;
    timer.Start();

    while(window.IsOpen())
    {
        float frameTime = static_cast<float>(timer.Stop());
        timer.Start();
        avgTime.Add(frameTime);
        float time = avgTime.Get();
        float fps = 1.0f / time;

        window.SetTitle("ABench - " + std::to_string(fps) + " FPS (" + std::to_string(time * 1000.0f) + " ms)");
        window.Update(frameTime);
        rend.Draw(scene, window.GetCamera(), frameTime);
    }

#if defined(_DEBUG) && defined(WIN32)
    system("PAUSE");
#endif

    rend.WaitForAll();

    return 0;
}
