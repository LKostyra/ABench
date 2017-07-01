#pragma once

#include "Prerequisites.hpp"
#include "Common/FBXFile.hpp"
#include "Object.hpp"
#include "Mesh.hpp"
#include "Material.hpp"


namespace ABench {
namespace Scene {

using ObjectCallback = std::function<void(const Object*)>;

template <typename T>
using GetResult = std::pair<T*, bool>; // .second is true when new material was created


class Scene
{
    template <typename T>
    using ResourceMap = std::unordered_map<std::string, std::unique_ptr<T>>;

    ABench::Common::FBXFile mFBXFile;
    std::vector<Object> mObjects;
    ResourceMap<Mesh> mMeshComponents;
    ResourceMap<Material> mMaterials;

    // getters per each component type (so, for each component map)
    GetResult<Component> GetMeshComponent(const std::string& name);

public:
    Scene();
    ~Scene();

    bool Init(const std::string& fbxFile = "");
    Object* CreateObject();
    GetResult<Component> GetComponent(ComponentType type, const std::string& name);
    GetResult<Material> GetMaterial(const std::string& name);
    void ForEachObject(ObjectCallback func) const;
};

} // namespace Scene
} // namespace ABench
