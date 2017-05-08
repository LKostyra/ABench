#pragma once

#include "Prerequisites.hpp"
#include "Common/FBXFile.hpp"
#include "Object.hpp"
#include "Mesh.hpp"


namespace ABench {
namespace Scene {

class Scene
{
    ABench::Renderer::Device* mDevicePtr;

    ABench::Common::FBXFile mFBXFile;
    std::vector<Object> mObjects;
    std::list<Mesh> mMeshComponents;

public:
    using ObjectCallback = std::function<void(const Object*)>;

    Scene();
    ~Scene();

    bool Init(ABench::Renderer::Device* devicePtr, const std::string& fbxFile = "");
    Object* CreateObject();
    Component* CreateComponent(ComponentType type);
    void ForEachObject(ObjectCallback func) const;
};

} // namespace Scene
} // namespace ABench
