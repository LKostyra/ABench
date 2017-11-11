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
    using ResourceCollection = std::list<std::unique_ptr<T>>;

    DevicePtr mDevice;
    ResourceCollection<Buffer> mBuffers;
    ResourceCollection<Texture> mTextures;

    ResourceManager();
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;
    ~ResourceManager();

public:
    static ResourceManager& Instance();

    bool Init(const DevicePtr& device);

    // TODO smart pointers? shared_ptr could be a good idea
    Buffer* GetBuffer(const BufferDesc& desc);
    Texture* GetTexture(const TextureDesc& desc);
};

} // namespace Renderer
} // namespace ABench
