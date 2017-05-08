#pragma once

#include "Common/Common.hpp"

namespace ABench {
namespace Common {

class FBXFile
{
    using TraverseCallback = std::function<void(FbxNode*)>;

    FbxManager* mFbxManager;
    FbxIOSettings* mFbxIOSettings;
    std::unique_ptr<FbxGeometryConverter> mFbxConverter;
    FbxImporter* mFbxImporter;
    FbxScene* mFbxScene;
    bool mIsOpened;

    // debugging info print functions/enum translators
    std::string GetAttributeTypeName(FbxNodeAttribute::EType type);
    std::string GetLightTypeName(FbxLight::EType type);
    void PrintNode(FbxNode* node, int tabs);

    void TraverseNode(TraverseCallback func, FbxNode* node);

public:
    FBXFile();
    ~FBXFile();

    bool Open(const std::string& path);
    void Traverse(TraverseCallback func);
    void Close();

    operator bool() const
    {
        return IsOpened();
    }

    ABENCH_INLINE bool IsOpened() const
    {
        return mIsOpened;
    }

    ABENCH_INLINE FbxGeometryConverter* GetConverter() const
    {
        return mFbxConverter.get();
    }
};

} // namespace Common
} // namespace ABench
