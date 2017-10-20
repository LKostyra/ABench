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
    // avoid reallocation - it could free our buffers
    mMeshes.reserve(modelDesc.materials.size());

    for (int i = 0; i < modelDesc.materials.size(); ++i)
    {
        mMeshes.emplace_back();
        Mesh& m = mMeshes.back();
        if (!m.Init(modelDesc.mesh, i))
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
