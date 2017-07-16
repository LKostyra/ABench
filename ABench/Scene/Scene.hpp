#pragma once

#include "Prerequisites.hpp"
#include "Common/FBXFile.hpp"
#include "Object.hpp"
#include "Mesh.hpp"
#include "Light.hpp"
#include "Material.hpp"


namespace ABench {
namespace Scene {

template <typename T>
using Callback = std::function<bool(const T*)>; // return false to stop iterating

template <typename T>
using GetResult = std::pair<T*, bool>; // .second is true when new material was created


class Scene
{
    template <typename T>
    using ResourceMap = std::unordered_map<std::string, std::unique_ptr<T>>;

    ABench::Common::FBXFile mFBXFile;
    std::vector<Object> mObjects;
    ResourceMap<Mesh> mMeshComponents;
    ResourceMap<Light> mLightComponents;
    ResourceMap<Material> mMaterials;

    // getters per each component type
    GetResult<Component> GetMeshComponent(const std::string& name);
    GetResult<Component> GetLightComponent(const std::string& name);

public:
    Scene();
    ~Scene();

    bool Init(const std::string& fbxFile = "");
    Object* CreateObject();
    GetResult<Component> GetComponent(ComponentType type, const std::string& name);
    GetResult<Material> GetMaterial(const std::string& name);
    void ForEachLight(Callback<Object> func) const;
    void ForEachObject(Callback<Object> func) const;
};

} // namespace Scene
} // namespace ABench
