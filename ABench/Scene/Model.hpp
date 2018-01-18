#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

#include "Math/AABB.hpp"


namespace ABench {
namespace Scene {

using MeshTraverseCallback = std::function<void(Mesh*)>;

struct ModelDesc
{
    FbxMesh* mesh;
    std::vector<Material*> materials;

    ModelDesc()
        : mesh(nullptr)
        , materials()
    {
    }
};

class Model final: public Component
{
    Math::Matrix mTransform;
    Math::Vector4 mPosition;
    Math::Vector4 mScale;
    std::vector<Mesh> mMeshes;
    Math::AABB mAABB;
    mutable bool mToRender;

    ABENCH_INLINE void UpdateTransform()
    {
        mTransform = Math::CreateTranslationMatrix(mPosition) * Math::CreateScaleMatrix(mScale);
    }

public:
    Model(const std::string& name);
    ~Model();

    bool Init(const ModelDesc& modelDesc);
    void ForEachMesh(MeshTraverseCallback callback);

    ABENCH_INLINE void SetPosition(float x, float y, float z)
    {
        SetPosition(Math::Vector4(x, y, z, 1.0f));
    }

    ABENCH_INLINE void SetPosition(const Math::Vector4& position)
    {
        mPosition = position;
        UpdateTransform();
    }

    ABENCH_INLINE void SetScale(float scaleX, float scaleY, float scaleZ)
    {
        SetScale(Math::Vector4(scaleX, scaleY, scaleZ, 1.0f));
    }

    ABENCH_INLINE void SetScale(const Math::Vector4& scale)
    {
        mScale = scale;
        UpdateTransform();
    }

    ABENCH_INLINE void SetToRender(bool toRender) const
    {
        mToRender = toRender;
    }

    ABENCH_INLINE Mesh* GetMesh(size_t i)
    {
        return &mMeshes[i];
    }

    ABENCH_INLINE const Math::Vector4& GetPosition() const
    {
        return mPosition;
    }

    ABENCH_INLINE const Math::Matrix& GetTransform() const
    {
        return mTransform;
    }

    ABENCH_INLINE bool ToRender() const
    {
        return mToRender;
    }

    ABENCH_INLINE const Math::AABB& GetAABB() const
    {
        return mAABB;
    }

    ComponentType GetType() const override
    {
        return ComponentType::Model;
    }
};

} // namespace Scene
} // namespace ABench
