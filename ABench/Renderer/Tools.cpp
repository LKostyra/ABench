#include "../PCH.hpp"
#include "Tools.hpp"

#include "Util.hpp"
#include "Extensions.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

Tools::Tools(const Device* device)
    : mDeviceRef(device)
{
}

Tools::~Tools()
{
    for (auto& pipeLayout: mPipelineLayouts)
        vkDestroyPipelineLayout(mDeviceRef->GetDevice(), pipeLayout, nullptr);
}

VkPipelineLayout Tools::CreatePipelineLayout(VkDescriptorSetLayout* sets, int setCount)
{
    VkPipelineLayout layout;

    VkPipelineLayoutCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pSetLayouts = sets;
    info.setLayoutCount = setCount;
    VkResult result = vkCreatePipelineLayout(mDeviceRef->GetDevice(), &info, nullptr, &layout);
    RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Pipeline Layout");

    mPipelineLayouts.push_back(layout);
    return layout;
}

} // namespace Renderer
} // namespace ABench
