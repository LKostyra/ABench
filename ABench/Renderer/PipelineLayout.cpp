#include "../PCH.hpp"
#include "PipelineLayout.hpp"
#include "Util.hpp"
#include "Extensions.hpp"

#include "Common/Common.hpp"

namespace ABench {
namespace Renderer {

PipelineLayout::PipelineLayout(const Device* device)
    : mDevicePtr(device)
    , mPipelineLayout(VK_NULL_HANDLE)
{
}

PipelineLayout::~PipelineLayout()
{
    if (mPipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(mDevicePtr->mDevice, mPipelineLayout, nullptr);
}

bool PipelineLayout::Init(const PipelineLayoutDesc& desc)
{
    VkPipelineLayoutCreateInfo info;
    ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    VkResult result = vkCreatePipelineLayout(mDevicePtr->mDevice, &info, nullptr, &mPipelineLayout);
    CHECK_VKRESULT(result, "Failed to create Pipeline Layout");

    return true;
}

} // namespace Renderer
} // namespace ABench
