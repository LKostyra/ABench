#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Mesh.hpp"
#include "Material.hpp"


namespace ABench {
namespace Scene {

class Model final: public Component
{
    std::string mName;
    std::vector<Mesh> mMeshes;

public:
    Model(const std::string& name);
    ~Model();

    bool Init(FbxMesh* mesh = nullptr);

    ABENCH_INLINE Mesh* GetMesh(size_t i)
    {
        return &mMeshes[i];
    }

    ABENCH_INLINE void SetMaterial(Material* mat)
    {
        mMeshes[0].SetMaterial(mat);
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
