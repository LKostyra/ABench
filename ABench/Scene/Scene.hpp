#pragma once

#include "Prerequisites.hpp"
#include "Common/FBXFile.hpp"
#include "Object.hpp"


namespace ABench {
namespace Scene {

class Scene
{
    ABench::Common::FBXFile mFBXFile;
    std::vector<Object> mObjects;

public:
    using ObjectIteratorFunction = std::function<void(const Object*)>;

    Scene();
    ~Scene();

    bool Init(const std::string fbxFile);
    Object* CreateObject();
    void ForEachObject(ObjectIteratorFunction func) const;
};

} // namespace Scene
} // namespace ABench
