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

BufferPtr ResourceManager::GetBuffer(const BufferDesc& desc)
{
    mBuffers.emplace_back(std::make_shared<Buffer>());
    BufferPtr& b = mBuffers.back();
    if (!b->Init(mDevice, desc))
    {
        mBuffers.pop_back();
        return nullptr;
    }

    return b;
}

TexturePtr ResourceManager::GetTexture(const TextureDesc& desc)
{
    mTextures.emplace_back(std::make_shared<Texture>());
    TexturePtr& t = mTextures.back();
    if (!t->Init(mDevice, desc))
    {
        mTextures.pop_back();
        return nullptr;
    }

    return t;
}

} // namespace Renderer
} // namespace ABench
