#pragma once

#include <fbxsdk.h>
#include <memory>
#include "Common/Common.hpp"

namespace ABench {
namespace Common {

class FBXFile
{
    FbxManager* mFbxManager;
    FbxIOSettings* mFbxIOSettings;
    std::unique_ptr<FbxGeometryConverter> mFbxConverter;
    FbxImporter* mFbxImporter;
    FbxScene* mFbxScene;
    bool mIsOpened;

    std::string GetAttributeTypeName(FbxNodeAttribute::EType type);
    void PrintNode(FbxNode* node, int tabs);

public:
    FBXFile();
    ~FBXFile();

    bool Open(const std::string& path);
    void Close();

    operator bool() const
    {
        return IsOpened();
    }

    ABENCH_INLINE bool IsOpened() const
    {
        return mIsOpened;
    }
};

} // namespace Common
} // namespace ABench
