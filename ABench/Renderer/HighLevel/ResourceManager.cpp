#include "PCH.hpp"
#include "ResourceManager.hpp"

namespace ABench {
namespace Renderer {

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

ResourceManager& ResourceManager::Instance()
{
    static ResourceManager instance;
    return instance;
}

bool ResourceManager::Init(const DevicePtr& device)
{
    mDevice = device;
    return true;
}

Buffer* ResourceManager::GetBuffer(const BufferDesc& desc)
{
    mBuffers.emplace_back(std::make_unique<Buffer>());
    Buffer* b = mBuffers.back().get();
    if (!b->Init(mDevice, desc))
    {
        mBuffers.pop_back();
        return nullptr;
    }

    return b;
}

Texture* ResourceManager::GetTexture(const TextureDesc& desc)
{
    mTextures.emplace_back(std::make_unique<Texture>());
    Texture* t = mTextures.back().get();
    if (!t->Init(mDevice, desc))
    {
        mTextures.pop_back();
        return nullptr;
    }

    return t;
}

} // namespace Renderer
} // namespace ABench
