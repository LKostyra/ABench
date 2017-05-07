#pragma once

#include "Prerequisites.hpp"
#include "Common/FBXFile.hpp"
#include "Object.hpp"
#include "Mesh.hpp"


namespace ABench {
namespace Scene {

class Scene
{
    ABench::Common::FBXFile mFBXFile;
    std::vector<Object> mObjects;
    std::vector<Mesh> mMeshComponents;

public:
    using ObjectCallback = std::function<void(const Object*)>;

    Scene();
    ~Scene();

    bool Init(const std::string fbxFile);
    Object* CreateObject();
    Component* CreateComponent(ComponentType type);
    void ForEachObject(ObjectCallback func) const;
};

} // namespace Scene
} // namespace ABench
