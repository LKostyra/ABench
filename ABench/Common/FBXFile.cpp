#include "PCH.hpp"
#include "FBXFile.hpp"
#include "Logger.hpp"


namespace ABench {
namespace Common {

FBXFile::FBXFile()
    : mFbxManager(nullptr)
    , mFbxIOSettings(nullptr)
    , mFbxConverter(nullptr)
    , mFbxImporter(nullptr)
    , mFbxScene(nullptr)
    , mIsOpened(false)
{
    mFbxManager = FbxManager::Create();
    mFbxIOSettings = FbxIOSettings::Create(mFbxManager, IOSROOT);
    mFbxManager->SetIOSettings(mFbxIOSettings);
    mFbxConverter.reset(new FbxGeometryConverter(mFbxManager));
}

FBXFile::~FBXFile()
{
    Close();
    mFbxConverter.reset();
    mFbxManager->Destroy();
}

std::string FBXFile::GetAttributeTypeName(FbxNodeAttribute::EType type)
{ 
    switch(type) { 
        case FbxNodeAttribute::eUnknown: return "unidentified"; 
        case FbxNodeAttribute::eNull: return "null"; 
        case FbxNodeAttribute::eMarker: return "marker"; 
        case FbxNodeAttribute::eSkeleton: return "skeleton"; 
        case FbxNodeAttribute::eMesh: return "mesh"; 
        case FbxNodeAttribute::eNurbs: return "nurbs"; 
        case FbxNodeAttribute::ePatch: return "patch"; 
        case FbxNodeAttribute::eCamera: return "camera"; 
        case FbxNodeAttribute::eCameraStereo: return "stereo"; 
        case FbxNodeAttribute::eCameraSwitcher: return "camera switcher"; 
        case FbxNodeAttribute::eLight: return "light"; 
        case FbxNodeAttribute::eOpticalReference: return "optical reference"; 
        case FbxNodeAttribute::eOpticalMarker: return "marker"; 
        case FbxNodeAttribute::eNurbsCurve: return "nurbs curve"; 
        case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface"; 
        case FbxNodeAttribute::eBoundary: return "boundary"; 
        case FbxNodeAttribute::eNurbsSurface: return "nurbs surface"; 
        case FbxNodeAttribute::eShape: return "shape"; 
        case FbxNodeAttribute::eLODGroup: return "lodgroup"; 
        case FbxNodeAttribute::eSubDiv: return "subdiv"; 
        default: return "unknown"; 
    } 
}

void FBXFile::PrintNode(FbxNode* node, int tabs)
{
    std::string start;
    for (int i = 0; i < tabs; ++i)
        start += "  ";

    LOGD(start << node->GetName());
    for (int i = 0; i < node->GetNodeAttributeCount(); i++)
    {
        FbxNodeAttribute* attr = node->GetNodeAttributeByIndex(i);
        LOGD(start << "-> " << GetAttributeTypeName(attr->GetAttributeType())
                   << ": " << attr->GetName());
        if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            LOGD(start << "   VertCount: " << node->GetMesh()->GetPolygonCount());
            LOGD(start << "   Layers: " << node->GetMesh()->GetLayerCount());
        }
    }

    for (int i = 0; i < node->GetChildCount(); ++i)
        PrintNode(node->GetChild(i), tabs+1);
}

bool FBXFile::Open(const std::string& path)
{
    if (mIsOpened)
        Close();

    mFbxImporter = FbxImporter::Create(mFbxManager, "");
    mFbxScene = FbxScene::Create(mFbxManager, "");

    if (!mFbxImporter->Initialize(path.c_str(), -1, mFbxManager->GetIOSettings()))
    {
        const char* error = mFbxImporter->GetStatus().GetErrorString();
        LOGE("Failed to open FBX file " << path << ": " << error);
        return false;
    }

    if (!mFbxImporter->IsFBX())
    {
        LOGE("File " << path << " is not an FBX Scene file");
        return false;
    }

    mFbxImporter->Import(mFbxScene);

    // print some details about our FBX file (debugging info only)
    FbxNode* root = mFbxScene->GetRootNode();
    if (root)
    {
        LOGD("FBX file structure:");
        PrintNode(root, 0);
    }

    LOGI("Opened FBX file " << path);
    mIsOpened = true;
    return true;
}

void FBXFile::Close()
{
    if (!mIsOpened)
        return;

    mFbxScene->Destroy();
    mFbxScene = nullptr;

    mFbxImporter->Destroy();
    mFbxImporter = nullptr;

    mIsOpened = false;
}

} // namespace Common
} // namespace ABench
