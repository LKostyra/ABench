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

bool Model::Init(FbxMesh* mesh)
{
    // temporary implementation
    mMeshes.emplace_back();
    return mMeshes.back().Init(mesh);
}

} // namespace Scene
} // namespace ABench
