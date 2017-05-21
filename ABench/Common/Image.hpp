#pragma once

#include "Common.hpp"

namespace ABench {
namespace Common {

class Image
{
    FIBITMAP* mBitmap;
    uint32_t mWidth;
    uint32_t mHeight;
    FREE_IMAGE_COLOR_TYPE mColorType;

public:
    Image();
    ~Image();

    bool Init(const std::string& path);

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

    ABENCH_INLINE void* GetData() const
    {
        return FreeImage_GetBits(mBitmap);
    }
};

} // namespace Common
} // namespace ABench
