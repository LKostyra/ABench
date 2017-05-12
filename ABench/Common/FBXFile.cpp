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
    // function copied from FBX manual/introduction
    switch (type)
    {
        case FbxNodeAttribute::eUnknown: return "unidentified"; 
        case FbxNodeAttribute::eNull: return "null"; 
        case FbxNodeAttribute::eMarker: return "marker"; 
        case FbxNodeAttribute::eSkeleton: return "skeleton"; 
        case FbxNodeAttribute::eMesh: return "mesh"; 
        case FbxNodeAttribute::eNurbs: return "nurbs"; 
        case FbxNodeAttribute::ePatch: return "patch"; 
        case FbxNodeAttribute::eCamera: return "camera"; 
        case FbxNodeAttribute::eCameraStereo: return "camera stereo"; 
        case FbxNodeAttribute::eCameraSwitcher: return "camera switcher"; 
        case FbxNodeAttribute::eLight: return "light"; 
        case FbxNodeAttribute::eOpticalReference: return "optical reference"; 
        case FbxNodeAttribute::eOpticalMarker: return "optical marker"; 
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

std::string FBXFile::GetLightTypeName(FbxLight::EType type)
{
    switch (type)
    {
    case FbxLight::ePoint: return "point";
    case FbxLight::eDirectional: return "directional";
    case FbxLight::eSpot: return "spot";
    case FbxLight::eArea: return "area";
    case FbxLight::eVolume: return "volume";
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
        
        if (attr->GetAttributeType() == FbxNodeAttribute::eLight)
        {
            LOGD(start << "   Type: " << GetLightTypeName(node->GetLight()->LightType.Get()));
        }
    }

    for (int i = 0; i < node->GetChildCount(); ++i)
        PrintNode(node->GetChild(i), tabs+1);
}

void FBXFile::TraverseNode(TraverseCallback func, FbxNode* node)
{
    func(node);

    uint32_t childCount = node->GetChildCount();
    for (uint32_t i = 0; i < childCount; ++i)
    {
        TraverseNode(func, node->GetChild(i));
    }
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

void FBXFile::Traverse(TraverseCallback func)
{
    FbxNode* root = mFbxScene->GetRootNode();

    if (root)
        TraverseNode(func, root);
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
