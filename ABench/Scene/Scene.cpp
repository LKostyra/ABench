#include "PCH.hpp"
#include "Scene.hpp"

#include "Common/Logger.hpp"

namespace ABench {
namespace Scene {

Scene::Scene()
{
}

Scene::~Scene()
{
}

FbxFileTexture* Scene::FileTextureFromMaterial(FbxSurfaceMaterial* material, const std::string& propertyName)
{
    FbxProperty prop = material->FindProperty(propertyName.c_str());
    if (!prop.IsValid())
    {
        LOGW("Material " << material->GetName() << " has no " << propertyName << " property");
        return nullptr;
    }

    FbxTexture* tex = prop.GetSrcObject<FbxTexture>(0);
    FbxFileTexture* texFile = FbxCast<FbxFileTexture>(tex);
    return texFile; // can return nullptr here if requested property does not exist
}

bool Scene::Init(const std::string& fbxFile)
{
    if (!fbxFile.empty())
    {
        LOGD("Loading scene from FBX file");

        if (!mFBXFile.Open(fbxFile))
        {
            LOGE("Failed to open FBX scene file.");
            return false;
        }

        uint32_t counter = 0;
        mFBXFile.Traverse([&](FbxNode* node) {
            uint32_t attributeCount = node->GetNodeAttributeCount();

            for (uint32_t i = 0; i < attributeCount; ++i)
            {
                FbxNodeAttribute* attr = node->GetNodeAttributeByIndex(i);

                if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
                {
                    FbxMesh* mesh = reinterpret_cast<FbxMesh*>(attr);
                    if (!mesh->IsTriangleMesh())
                    {
                        LOGD("Mesh " << node->GetName() << " requires triangulation - converting, this might take some time.");
                        mFBXFile.GetConverter()->Triangulate(attr, true);
                    }

                    ModelDesc modelDesc;
                    modelDesc.mesh = mesh;
                    // Material loading
                    int32_t matCount = node->GetSrcObjectCount<FbxSurfaceMaterial>();
                    Material* mat = nullptr;
                    if (matCount > 0)
                    {
                        for (int32_t m = 0; m < matCount; ++m)
                        {
                            FbxSurfaceMaterial* material = node->GetSrcObject<FbxSurfaceMaterial>(m);
                            FbxFileTexture* diffTex = FileTextureFromMaterial(material, material->sDiffuse);
                            FbxFileTexture* normTex = FileTextureFromMaterial(material, material->sNormalMap);
                            FbxFileTexture* maskTex = FileTextureFromMaterial(material, material->sTransparencyFactor);

                            auto matResult = GetMaterial(material->GetName());
                            mat = matResult.first;
                            if (matResult.second)
                            {
                                // new material, initialize
                                MaterialDesc matDesc;
                                if (diffTex) matDesc.diffusePath = diffTex->GetFileName();
                                if (normTex) matDesc.normalPath = normTex->GetFileName();
                                if (maskTex) matDesc.maskPath = maskTex->GetFileName();
                                if (!mat->Init(matDesc))
                                {
                                    LOGE("Failed to initialize material for mesh " << node->GetName());
                                    mat = nullptr;
                                }
                            }

                            modelDesc.materials.push_back(mat);
                        }
                    }

                    Object* o = CreateObject();
                    auto mResult = GetComponent(ComponentType::Model, node->GetName());
                    Model* m = dynamic_cast<Model*>(mResult.first);
                    if (mResult.second)
                        m->Init(modelDesc);

                    o->SetComponent(m);
                    o->SetScale(static_cast<float>(node->LclScaling.Get()[0]),
                                static_cast<float>(node->LclScaling.Get()[1]),
                                static_cast<float>(node->LclScaling.Get()[2]));
                    o->SetPosition(static_cast<float>(node->LclTranslation.Get()[0]),
                                   static_cast<float>(node->LclTranslation.Get()[1]),
                                   static_cast<float>(node->LclTranslation.Get()[2]));
                    counter += matCount;
                }
            }
        });

        LOGD("Counted " << counter << " objects in scene " << fbxFile);
    }
    else
        LOGI("Initialized empty scene");

    return true;
}

Object* Scene::CreateObject()
{
    mObjects.emplace_back();
    return &mObjects.back();
}

GetResult<Component> Scene::GetModelComponent(const std::string& name)
{
    bool created = false;
    auto mesh = mModelComponents.find(name);
    if (mesh == mModelComponents.end())
    {
        mesh = mModelComponents.insert(std::make_pair(name, std::make_unique<Model>(name))).first;
        created = true;
    }

    return std::make_pair(mesh->second.get(), created);
}

GetResult<Component> Scene::GetLightComponent(const std::string& name)
{
    bool created = false;
    auto light = mLightComponents.find(name);
    if (light == mLightComponents.end())
    {
        light = mLightComponents.insert(std::make_pair(name, std::make_unique<Light>(name))).first;
        created = true;
    }

    return std::make_pair(light->second.get(), created);
}

GetResult<Component> Scene::GetComponent(ComponentType type, const std::string& name)
{
    switch (type)
    {
    case ComponentType::Model:
        return GetModelComponent(name);
    case ComponentType::Light:
        return GetLightComponent(name);
    default:
        LOGE("Unknown component type provided to get");
        return std::make_pair(nullptr, false);
    }
}

GetResult<Material> Scene::GetMaterial(const std::string& name)
{
    bool created = false;
    auto mat = mMaterials.find(name);
    if (mat == mMaterials.end())
    {
        mat = mMaterials.insert(std::make_pair(name, std::make_unique<Material>(name))).first;
        created = true;
    }

    return std::make_pair(mat->second.get(), created);
}

void Scene::ForEachLight(Callback<Object> func) const
{
    for (auto& l: mObjects)
        if (l.GetComponent()->GetType() == ComponentType::Light)
            if (!func(&l))
                return;
}

void Scene::ForEachObject(Callback<Object> func) const
{
    for (auto& o: mObjects)
        if (!func(&o))
            return;
}

} // namespace Scene
} // namespace ABench
