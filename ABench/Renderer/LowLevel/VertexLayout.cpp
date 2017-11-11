#include "PCH.hpp"
#include "VertexLayout.hpp"
#include "Util.hpp"
#include "Translations.hpp"

#include "Common/Common.hpp"

#include <list>
#include <tuple>


namespace ABench {
namespace Renderer {

VertexLayout::VertexLayout()
{
}

VertexLayout::~VertexLayout()
{
}

bool VertexLayout::Init(const VertexLayoutDesc& desc)
{
    VkVertexInputBindingDescription vibDesc;
    ZERO_MEMORY(vibDesc);

    VkVertexInputAttributeDescription viaDesc;
    ZERO_MEMORY(viaDesc);

    // gather how much VBs we have to bind
    std::list<std::tuple<int, int>> vbList;
    for (uint32_t i = 0; i < desc.entryCount; ++i)
        vbList.emplace_back(i, desc.entries[i].binding);

    // sort according to VB ID
    vbList.sort([](const std::tuple<int, int>& first, const std::tuple<int, int>& second)
    {
        return (std::get<1>(first) < std::get<1>(second));
    });

    // remove spare VB IDs
    vbList.unique([](const std::tuple<int, int>& first, const std::tuple<int, int>& second)
    {
        return (std::get<1>(first) == std::get<1>(second));
    });

    // create bindings
    for (auto& vb : vbList)
    {
        vibDesc.binding = std::get<1>(vb);
        vibDesc.inputRate = desc.entries[std::get<0>(vb)].instance ? VK_VERTEX_INPUT_RATE_INSTANCE
                                                                   : VK_VERTEX_INPUT_RATE_VERTEX;

        vibDesc.stride = desc.entries[std::get<0>(vb)].stride;

        mBindings.push_back(vibDesc);
    }

    // create attributes
    for (uint32_t i = 0; i < desc.entryCount; ++i)
    {
        viaDesc.location = i;
        viaDesc.binding = desc.entries[i].binding;
        viaDesc.format = desc.entries[i].format;
        viaDesc.offset = desc.entries[i].offset;
        mAttributes.push_back(viaDesc);
    }

    return true;
}

} // namespace Renderer
} // namespace ABench
