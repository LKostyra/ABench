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

    // TODO replace path shenaningans with FS class
    // TODO cast "forward-slashify" on these pesky slashes in path
    size_t trailingSlash = path.rfind('\\');
    std::string newPath;
    if (trailingSlash != std::string::npos)
    {
        newPath = path.substr(trailingSlash + 1);
        newPath = "Data/Textures/" + newPath;
    }
    else
    {
        newPath = path;
    }

    fif = FreeImage_GetFileType(newPath.c_str());
    if (fif == FIF_UNKNOWN)
        fif = FreeImage_GetFIFFromFilename(newPath.c_str());

    if (fif == FIF_UNKNOWN)
    {
        LOGE("Cannot recognize image type of file " << newPath);
        return false;
    }

    FIBITMAP* bitmap = FreeImage_Load(fif, newPath.c_str());
    if (!bitmap)
    {
        LOGE("Failed to load file " << newPath);
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
