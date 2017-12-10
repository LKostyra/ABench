#pragma once

#include "Prerequisites.hpp"
#include "Renderer/LowLevel/Device.hpp"
#include "Renderer/LowLevel/Buffer.hpp"
#include "Renderer/LowLevel/Texture.hpp"


namespace ABench {
namespace Renderer {

class ResourceManager final
{
    template <typename T>
    using ResourceCollection = std::list<T>;

    DevicePtr mDevice;
    ResourceCollection<BufferPtr> mBuffers;
    ResourceCollection<TexturePtr> mTextures;

    ResourceManager();
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;
    ~ResourceManager();

public:
    static ResourceManager& Instance();

    bool Init(const DevicePtr& device);

    BufferPtr GetBuffer(const BufferDesc& desc);
    TexturePtr GetTexture(const TextureDesc& desc);
};

} // namespace Renderer
} // namespace ABench
