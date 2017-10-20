#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Mesh.hpp"
#include "Material.hpp"


namespace ABench {
namespace Scene {

using MeshTraverseCallback = std::function<void(Mesh*)>;

struct ModelDesc
{
    FbxMesh* mesh;
    std::vector<Material*> materials;

    ModelDesc::ModelDesc()
        : mesh(nullptr)
        , materials()
    {
    }
};

class Model final: public Component
{
    std::string mName;
    std::vector<Mesh> mMeshes;

public:
    Model(const std::string& name);
    ~Model();

    bool Init(const ModelDesc& modelDesc);
    void ForEachMesh(MeshTraverseCallback callback);

    ABENCH_INLINE Mesh* GetMesh(size_t i)
    {
        return &mMeshes[i];
    }

    ABENCH_INLINE std::string GetName() const
    {
        return mName;
    }

    ComponentType GetType() const override
    {
        return ComponentType::Model;
    }
};

} // namespace Scene
} // namespace ABench
