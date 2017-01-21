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

    VertexLayoutEntry()
        : format(VK_FORMAT_UNDEFINED)
        , offset(0)
        , binding(0)
        , stride(0)
        , instance(false)
    {
    }

    VertexLayoutEntry(VkFormat format, uint32_t offset, uint32_t binding, uint32_t stride, bool instance)
        : format(format)
        , offset(offset)
        , binding(binding)
        , stride(stride)
        , instance(instance)
    {
    }
};

struct VertexLayoutDesc
{
    VertexLayoutEntry* entries;
    uint32_t entryCount;

    VertexLayoutDesc()
        : entries(nullptr)
        , entryCount(0)
    {
    }
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
