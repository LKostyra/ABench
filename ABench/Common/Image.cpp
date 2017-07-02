#include "PCH.hpp"
#include "Image.hpp"
#include "Logger.hpp"

#include "Common/FS.hpp"
#include "ResourceDir.hpp"


namespace ABench {
namespace Common {

Image::Image()
    : mWidth(0)
    , mHeight(0)
{
}

Image::~Image()
{
    for (auto& b: mBitmaps)
        FreeImage_Unload(b);
}

bool Image::GenerateMipmaps()
{
    uint32_t width = mWidth;
    uint32_t height = mHeight;

    FIBITMAP* b;
    while (width > 1 && height > 1)
    {
        width >>= 1; height >>= 1;
        b = FreeImage_Rescale(mBitmaps.back(), width, height);
        mBitmaps.push_back(b);
    }

    return true;
}

bool Image::Init(const std::string& path, bool generateMipmaps)
{
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

    std::string newPath = FS::JoinPaths(ResourceDir::TEXTURES, FS::GetFileName(FS::UnifySlashes(path)));

    fif = FreeImage_GetFileType(newPath.c_str());
    if (fif == FIF_UNKNOWN)
        fif = FreeImage_GetFIFFromFilename(newPath.c_str());

    if (fif == FIF_UNKNOWN)
    {
        LOGE("Cannot recognize image type of file " << newPath);
        return false;
    }

    FIBITMAP* loadedBitmap = FreeImage_Load(fif, newPath.c_str());
    if (!loadedBitmap)
    {
        LOGE("Failed to load file " << newPath);
        return false;
    }

    FIBITMAP* bitmap = nullptr;
    if (FreeImage_GetColorType(loadedBitmap) == FIC_RGB)
    {
        bitmap = FreeImage_ConvertTo32Bits(loadedBitmap);
        FreeImage_Unload(loadedBitmap);
    }
    else
        bitmap = loadedBitmap;

    mBitmaps.push_back(bitmap);
    mWidth = FreeImage_GetWidth(mBitmaps[0]);
    mHeight = FreeImage_GetHeight(mBitmaps[0]);
    mColorType = FIC_RGBALPHA;

    if (generateMipmaps)
        if (!GenerateMipmaps())
            LOGW("Failed to generate mipmaps for image " << newPath);

    return true;
}

} // namespace Common
} // namespace ABench
