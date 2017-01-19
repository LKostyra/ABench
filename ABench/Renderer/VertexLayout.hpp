#pragma once

#include <vector>


namespace ABench {
namespace Renderer {

struct VertexLayoutEntry
{
    VkFormat format;
    uint32_t offset;
    uint32_t binding;
    uint32_t stride;
    bool instance;
};

struct VertexLayoutDesc
{
    VertexLayoutEntry* entries;
    uint32_t entryCount;
};

class VertexLayout
{
    friend class Pipeline;

    std::vector<VkVertexInputBindingDescription> mBindings;
    std::vector<VkVertexInputAttributeDescription> mAttributes;

public:
    VertexLayout();
    ~VertexLayout();

    bool Init(const VertexLayoutDesc& desc);
};

} // namespace Renderer
} // namespace ABench
