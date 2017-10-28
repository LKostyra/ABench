#include "PCH.hpp"
#include "Model.hpp"

#include "Common/Common.hpp"
#include "Common/Logger.hpp"

using namespace ABench::Renderer;

namespace ABench {
namespace Scene {

Model::Model(const std::string& name)
    : mName(name)
{
}

Model::~Model()
{
}

bool Model::Init(const ModelDesc& modelDesc)
{
    if (modelDesc.materials.size() == 0)
    {
        LOGE("A material is required to create a model");
        return false;
    }

    // avoid reallocation - it could free our buffers
    mMeshes.reserve(modelDesc.materials.size());

    for (size_t i = 0; i < modelDesc.materials.size(); ++i)
    {
        mMeshes.emplace_back();
        Mesh& m = mMeshes.back();
        if (!m.Init(modelDesc.mesh, static_cast<uint32_t>(i)))
        {
            LOGE("Failed to create mesh " << i << " for model " << mName);
            return false;
        }
        m.SetMaterial(modelDesc.materials[i]);
    }

    return true;
}

void Model::ForEachMesh(MeshTraverseCallback callback)
{
    for (auto& m: mMeshes)
        callback(&m);
}

} // namespace Scene
} // namespace ABench
