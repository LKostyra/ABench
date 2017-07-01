#pragma once

#include "Common.hpp"

namespace ABench {
namespace Common {

class Image
{
    std::vector<FIBITMAP*> mBitmaps;
    uint32_t mWidth;
    uint32_t mHeight;
    FREE_IMAGE_COLOR_TYPE mColorType;

    bool GenerateMipmaps();

public:
    Image();
    ~Image();

    bool Init(const std::string& path, bool generateMipmaps = false);

    ABENCH_INLINE uint32_t GetWidth() const
    {
        return mWidth;
    }

    ABENCH_INLINE uint32_t GetHeight() const
    {
        return mHeight;
    }

    ABENCH_INLINE uint32_t GetColorType() const
    {
        return mColorType;
    }

    ABENCH_INLINE void* GetSubimageData(uint32_t mipmapLevel) const
    {
        return FreeImage_GetBits(mBitmaps[mipmapLevel]);
    }

    ABENCH_INLINE uint32_t GetSubimageSize(uint32_t mipmapLevel) const
    {
        uint32_t bytePerPixel = FreeImage_GetBPP(mBitmaps[mipmapLevel]) / 8;
        return FreeImage_GetWidth(mBitmaps[mipmapLevel]) * FreeImage_GetHeight(mBitmaps[mipmapLevel]) * bytePerPixel;
    }

    ABENCH_INLINE uint32_t GetMipmapCount() const
    {
        return static_cast<uint32_t>(mBitmaps.size());
    }
};

} // namespace Common
} // namespace ABench
