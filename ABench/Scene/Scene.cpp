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

        mFBXFile.Traverse([&](FbxNode* node) {
            uint32_t attributeCount = node->GetNodeAttributeCount();

            for (uint32_t i = 0; i < attributeCount; ++i)
            {
                FbxNodeAttribute* attr = node->GetNodeAttributeByIndex(i);

                if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
                {
                    if (!node->GetMesh()->IsTriangleMesh())
                    {
                        LOGD("Mesh " << node->GetName() << " requires triangulation - converting, this might take some time.");
                        mFBXFile.GetConverter()->Triangulate(attr, true);
                    }

                    // check if we have materials
                    int32_t matCount = node->GetSrcObjectCount<FbxSurfaceMaterial>();
                    Material* mat = nullptr;
                    if (matCount > 0)
                    {
                        // TODO here we might want to extract more than one material!
                        FbxSurfaceMaterial* material = node->GetSrcObject<FbxSurfaceMaterial>(0);
                        FbxFileTexture* diffTex = FileTextureFromMaterial(material, material->sDiffuse);
                        FbxFileTexture* normTex = FileTextureFromMaterial(material, material->sNormalMap);

                        auto matResult = GetMaterial(material->GetName());
                        mat = matResult.first;
                        if (matResult.second)
                        {
                            // new material, initialize
                            MaterialDesc matDesc;
                            if (diffTex) matDesc.diffusePath = diffTex->GetFileName();
                            if (normTex) matDesc.normalPath = normTex->GetFileName();
                            if (!mat->Init(matDesc))
                            {
                                LOGE("Failed to initialize material for mesh " << node->GetName());
                                mat = nullptr;
                            }
                        }
                    }

                    Object* o = CreateObject();
                    auto mResult = GetComponent(ComponentType::Mesh, node->GetName());
                    Mesh* m = dynamic_cast<Mesh*>(mResult.first);

                    if (mResult.second)
                        m->Init(node->GetMesh());

                    m->SetMaterial(mat);
                    o->SetComponent(m);
                    o->SetScale(static_cast<float>(node->LclScaling.Get()[0]),
                                static_cast<float>(node->LclScaling.Get()[1]),
                                static_cast<float>(node->LclScaling.Get()[2]));
                    o->SetPosition(static_cast<float>(node->LclTranslation.Get()[0]),
                                   static_cast<float>(node->LclTranslation.Get()[1]),
                                   static_cast<float>(node->LclTranslation.Get()[2]));
                }
            }
        });
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

GetResult<Component> Scene::GetMeshComponent(const std::string& name)
{
    bool created = false;
    auto mesh = mMeshComponents.find(name);
    if (mesh == mMeshComponents.end())
    {
        mesh = mMeshComponents.insert(std::make_pair(name, std::make_unique<Mesh>(name))).first;
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
    case ComponentType::Mesh:
        return GetMeshComponent(name);
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
