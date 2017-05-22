#include "PCH.hpp"
#include "Image.hpp"
#include "Logger.hpp"


namespace ABench {
namespace Common {

Image::Image()
    : mBitmap(nullptr)
    , mWidth(0)
    , mHeight(0)
{
}

Image::~Image()
{
    if (mBitmap)
        FreeImage_Unload(mBitmap);
}

bool Image::Init(const std::string& path)
{
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

    fif = FreeImage_GetFileType(path.c_str());
    if (fif == FIF_UNKNOWN)
        fif = FreeImage_GetFIFFromFilename(path.c_str());

    if (fif == FIF_UNKNOWN)
    {
        LOGE("Cannot recognize image type of file " << path);
        return false;
    }


    FIBITMAP* bitmap = FreeImage_Load(fif, path.c_str());
    if (!bitmap)
    {
        LOGE("Error while loading file " << path);
        return false;
    }

    if (FreeImage_GetColorType(bitmap) == FIC_RGB)
    {
        mBitmap = FreeImage_ConvertTo32Bits(bitmap);
        FreeImage_Unload(bitmap);
    }
    else
        mBitmap = bitmap;

    mWidth = FreeImage_GetWidth(mBitmap);
    mHeight = FreeImage_GetHeight(mBitmap);
    mColorType = FIC_RGBALPHA;

    return true;
}

} // namespace Common
} // namespace ABench
