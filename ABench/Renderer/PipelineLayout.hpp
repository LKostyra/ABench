#pragma once

#include "Device.hpp"

namespace ABench {
namespace Renderer {

struct PipelineLayoutDesc
{

};

class PipelineLayout
{
    friend class Pipeline;

    const Device* mDevicePtr;

    VkPipelineLayout mPipelineLayout;

public:
    PipelineLayout(const Device* device);
    ~PipelineLayout();

    bool Init(const PipelineLayoutDesc& desc);
};

} // namespace Renderer
} // namespace ABench
