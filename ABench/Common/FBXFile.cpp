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
    , mNodeCount(0)
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

void FBXFile::InitialTraverseNode(FbxNode* node, int printTabs)
{
    std::string start;
    for (int i = 0; i < printTabs; ++i)
        start += "  ";

    mNodeCount++;

    /*LOGD(start << node->GetName());
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
    }*/

    for (int i = 0; i < node->GetChildCount(); ++i)
        InitialTraverseNode(node->GetChild(i), printTabs + 1);
}

void FBXFile::TraverseNode(TraverseCallback func, FbxNode* node)
{
    func(node);
    mCurrentNode++;

    if (mNodeCount > 100)
    {
        if ((static_cast<float>(mCurrentNode) / static_cast<float>(mNodeCount)) >= mLastHitRatio)
        {
            LOGI("  Nodes processed: " << mCurrentNode << "/" << mNodeCount);
            mLastHitRatio += 0.1f;
            if (mLastHitRatio > 1.0f)
                mLastHitRatio = 1.0f;
        }
    }

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

    if (path.rfind(".fbx") == std::string::npos)
    {
        LOGE("Provided file is not an FBX file");
        return false;
    }

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

    if (!mFbxConverter->SplitMeshesPerMaterial(mFbxScene, true))
        LOGW("Failed to split meshes according to material");

    // print some details about our FBX file (debugging info only)
    FbxNode* root = mFbxScene->GetRootNode();
    if (root)
    {
        LOGD("FBX file structure:");
        InitialTraverseNode(root, 0);
    }

    // no validation here - on this stage the path should be valid (scene is loaded from file)
    size_t filenameStart = path.rfind('/');
    size_t extensionStart = path.rfind('.');
    size_t filenameLength = extensionStart - filenameStart - 1;

    LOGD(path.substr(filenameStart + 1, filenameLength));
    mFbxScene->SetName(path.substr(filenameStart + 1, filenameLength).c_str());

    // scale to meter units
    if(mFbxScene->GetGlobalSettings().GetSystemUnit() == FbxSystemUnit::cm)
    {
        LOGI("Converting scene to meter unit system");
        const FbxSystemUnit::ConversionOptions lConversionOptions = {
            false, /* mConvertRrsNodes */
            true, /* mConvertAllLimits */
            true, /* mConvertClusters */
            true, /* mConvertLightIntensity */
            true, /* mConvertPhotometricLProperties */
            true  /* mConvertCameraClipPlanes */
        };

        // Convert the scene to meters using the defined options.
        FbxSystemUnit::m.ConvertScene(mFbxScene, lConversionOptions);
    }

    LOGI("Opened FBX file " << path << " (" << mNodeCount << " nodes)");
    mIsOpened = true;
    return true;
}

void FBXFile::Traverse(TraverseCallback func)
{
    FbxNode* root = mFbxScene->GetRootNode();
    LOGI("Traversing " << mFbxScene->GetName() << " scene (" << mNodeCount << " nodes)...");

    mCurrentNode = 0;
    mLastHitRatio = 0.1f;

    if (root)
        TraverseNode(func, root);

    LOGI(mFbxScene->GetName() << " scene traversed.");
}

void FBXFile::Close()
{
    if (!mIsOpened)
        return;

    mFbxScene->Destroy();
    mFbxScene = nullptr;

    mFbxImporter->Destroy();
    mFbxImporter = nullptr;

    mNodeCount = 0;
    mIsOpened = false;
}

} // namespace Common
} // namespace ABench
