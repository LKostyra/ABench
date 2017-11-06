#include "PCH.hpp"
#include "GridFrustumsGenerator.hpp"
#include "Tools.hpp"
#include "Device.hpp"
#include "Math/Plane.hpp"


extern ABench::Renderer::Device* gDevice;

namespace {

const uint32_t PIXELS_PER_GRID_FRUSTUM = 16;

} // namespace

namespace ABench {
namespace Renderer {

GridFrustumsGenerator::GridFrustumsGenerator()
    : mFrustumsPerWidth(0)
    , mFrustumsPerHeight(0)
    , mGridFrustumsData()
{
}

GridFrustumsGenerator::~GridFrustumsGenerator()
{
    if (mPipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(gDevice->GetDevice(), mPipelineLayout, nullptr);
}

bool GridFrustumsGenerator::Init(const GridFrustumsGeneratorDesc& desc)
{
    mGridFrustumsDataDesc.data = nullptr;
    mGridFrustumsDataDesc.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    mGridFrustumsDataDesc.type = BufferType::Dynamic;

    mPipelineLayout = Tools::CreatePipelineLayout();
    if (mPipelineLayout == VK_NULL_HANDLE)
        return false;

    MultiComputePipelineDesc cpDesc;
    cpDesc.computeShader.path = "GridFrustumGenerator.comp";
    cpDesc.pipelineDesc.pipelineLayout = mPipelineLayout;
    if (!mPipeline.Init(cpDesc))
        return false;

    return true;
}

bool GridFrustumsGenerator::Generate(uint32_t viewportWidth, uint32_t viewportHeight)
{
    mFrustumsPerWidth = viewportWidth / PIXELS_PER_GRID_FRUSTUM;
    mFrustumsPerHeight = viewportHeight / PIXELS_PER_GRID_FRUSTUM;

    if (viewportWidth % PIXELS_PER_GRID_FRUSTUM > 0)
        mFrustumsPerWidth++;
    if (viewportHeight % PIXELS_PER_GRID_FRUSTUM > 0)
        mFrustumsPerHeight++;

    mGridFrustumsDataDesc.dataSize = sizeof(Math::Plane) * mFrustumsPerWidth * mFrustumsPerHeight;
    if (!mGridFrustumsData.Init(mGridFrustumsDataDesc)) // LKTODO Buffer does not support releasing
        return false;

    return true;
}

} // namespace Renderer
} // namespace ABench
