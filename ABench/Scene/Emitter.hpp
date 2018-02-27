#pragma once

#include "Prerequisites.hpp"
#include "Component.hpp"

#include "Math/Vector.hpp"


namespace ABench {
namespace Scene {

ABENCH_ALIGN(16)
struct EmitterData
{
    Math::Vector4 pos;
    uint32_t particleLimit;
    float spawnPeriod;
    float spawnTimer;
    float lifeTime;
    mutable uint32_t lastSpawnedParticle;
    mutable uint32_t particleDataOffset;

    EmitterData()
        : pos()
        , particleLimit(0)
        , spawnPeriod(0.0f)
        , spawnTimer(0.0f)
        , lifeTime(0.0f)
        , particleDataOffset(0)
        , lastSpawnedParticle(0)
    {
    }

    EmitterData(const Math::Vector4& p, uint32_t limit, float period, float timer)
        : pos(p)
        , particleLimit(limit)
        , spawnPeriod(period)
        , spawnTimer(timer)
        , particleDataOffset(0)
        , lastSpawnedParticle(0)
    {
    }
};

struct EmitterDesc
{
    Math::Vector4 pos;
    uint32_t particleLimit;
    float spawnPeriod;

    EmitterDesc()
        : pos()
        , particleLimit(0)
        , spawnPeriod(0.0f)
    {
    }

    EmitterDesc(const Math::Vector4& p, uint32_t limit, float period)
        : pos(p)
        , particleLimit(limit)
        , spawnPeriod(period)
    {
    }
};

class Emitter: public Component
{
    friend class Renderer::ParticleEngine;

    EmitterData mData;

    ABENCH_INLINE void SetParticleDataOffset(uint32_t offset) const
    {
        mData.particleDataOffset = offset;
    }

public:
    Emitter(const std::string& name);
    ~Emitter();

    void Init(const EmitterDesc& desc);
    ABENCH_INLINE void* operator new (std::size_t size)
    {
        ABENCH_RETURN_ALIGNED_MALLOC(size, 16);
    }

    ABENCH_INLINE void operator delete(void* ptr)
    {
        ABENCH_ALIGNED_FREE(ptr);
    }

    ABENCH_INLINE void SetPosition(const Math::Vector4& pos)
    {
        mData.pos = pos;
    }

    ABENCH_INLINE void SetSpawnPeriod(float spawnPeriod)
    {
        mData.spawnPeriod = spawnPeriod;
    }

    ABENCH_INLINE void SetLifeTime(float time)
    {
        mData.lifeTime = time;
    }

    ABENCH_INLINE void SetParticleLimit(uint32_t particleLimit)
    {
        mData.particleLimit = particleLimit;
    }

    ABENCH_INLINE const EmitterData& GetData() const
    {
        return mData;
    }

    ABENCH_INLINE uint32_t GetParticleLimit() const
    {
        return mData.particleLimit;
    }

    ABENCH_INLINE uint32_t GetParticleDataOffset() const
    {
        return mData.particleDataOffset;
    }

    ABENCH_INLINE ComponentType GetType() const
    {
        return ComponentType::Emitter;
    }
};

using EmitterCollection = std::list<const Emitter*>;

} // namespace Scene
} // namespace ABench
