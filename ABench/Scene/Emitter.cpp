#include "PCH.hpp"
#include "Emitter.hpp"


namespace ABench {
namespace Scene {

Emitter::Emitter(const std::string& name)
    : Component(name)
    , mData()
{
}

Emitter::~Emitter()
{
}

void Emitter::Init(const EmitterDesc& desc)
{
    mData.pos = desc.pos;
    mData.particleLimit = desc.particleLimit;
    mData.spawnPeriod = desc.spawnPeriod;
    mData.spawnTimer = 0.0f;
    mData.particleDataOffset = 0;
}

} // namespace Scene
} // namespace ABench
