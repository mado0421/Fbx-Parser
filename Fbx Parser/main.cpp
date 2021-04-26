#include <windows.h>
#include <iostream>
#include <algorithm>
#include <DirectXMath.h>
#include <fbxsdk.h>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <fstream>

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(pManager->GetIOSettings()))
#endif

using namespace DirectX;
using namespace std;

struct Bone {
    FbxString           name;
    int                 parentIdx;
    FbxNode* node;
    FbxMatrix           globalMatrix;
    set<FbxTime>        keytimes;
    vector<FbxAMatrix>  transforms;
};

vector<Bone> g_vecBones;


// Initialize
void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
{
    //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
    pManager = FbxManager::Create();
    if (!pManager)
    {
        FBXSDK_printf("Error: Unable to create FBX Manager!\n");
        exit(1);
    }
    else FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

    //Create an IOSettings object. This object holds all import/export settings.
    FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
    pManager->SetIOSettings(ios);

    //Load plugins from the executable directory (optional)
    FbxString lPath = FbxGetApplicationDirectory();
    pManager->LoadPluginsDirectory(lPath.Buffer());

    //Create an FBX scene. This object holds most objects imported/exported from/to files.
    pScene = FbxScene::Create(pManager, "My Scene");
    if (!pScene)
    {
        FBXSDK_printf("Error: Unable to create FBX scene!\n");
        exit(1);
    }
}
bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename)
{
    int lFileMajor, lFileMinor, lFileRevision;
    int lSDKMajor, lSDKMinor, lSDKRevision;
    //int lFileFormat = -1;
    int lAnimStackCount;
    bool lStatus;
    char lPassword[1024];

    // Get the file version number generate by the FBX SDK.
    FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

    // Create an importer.
    FbxImporter* lImporter = FbxImporter::Create(pManager, "");

    // Initialize the importer by providing a filename.
    const bool lImportStatus = lImporter->Initialize(pFilename, -1, pManager->GetIOSettings());
    lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

    if (!lImportStatus)
    {
        FbxString error = lImporter->GetStatus().GetErrorString();
        FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
        FBXSDK_printf("Error returned: %s\n\n", error.Buffer());

        if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
        {
            FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
            FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
        }

        return false;
    }

    FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

    if (lImporter->IsFBX())
    {
        FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);

        // From this point, it is possible to access animation stack information without
        // the expense of loading the entire file.

        FBXSDK_printf("Animation Stack Information\n");

        lAnimStackCount = lImporter->GetAnimStackCount();

        FBXSDK_printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
        FBXSDK_printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
        FBXSDK_printf("\n");

        for (int i = 0; i < lAnimStackCount; i++)
        {
            FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

            FBXSDK_printf("    Animation Stack %d\n", i);
            FBXSDK_printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
            FBXSDK_printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

            // Change the value of the import name if the animation stack should be imported 
            // under a different name.
            FBXSDK_printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

            // Set the value of the import state to false if the animation stack should be not
            // be imported. 
            FBXSDK_printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
            FBXSDK_printf("\n");
        }

        // Set the import states. By default, the import states are always set to 
        // true. The code below shows how to change these states.
        IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
        IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
        IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
        IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
        IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
        IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
        IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
    }

    // Import the scene.
    lStatus = lImporter->Import(pScene);
    if (lStatus == false && lImporter->GetStatus() == FbxStatus::ePasswordError)
    {
        FBXSDK_printf("Please enter password: ");

        lPassword[0] = '\0';

        FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
            scanf("%s", lPassword);
        FBXSDK_CRT_SECURE_NO_WARNING_END

            FbxString lString(lPassword);

        IOS_REF.SetStringProp(IMP_FBX_PASSWORD, lString);
        IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

        lStatus = lImporter->Import(pScene);

        if (lStatus == false && lImporter->GetStatus() == FbxStatus::ePasswordError)
        {
            FBXSDK_printf("\nPassword is wrong, import aborted.\n");
        }
    }

    if (!lStatus || (lImporter->GetStatus() != FbxStatus::eSuccess))
    {
        FBXSDK_printf("********************************************************************************\n");
        if (lStatus)
        {
            FBXSDK_printf("WARNING:\n");
            FBXSDK_printf("   The importer was able to read the file but with errors.\n");
            FBXSDK_printf("   Loaded scene may be incomplete.\n\n");
        }
        else
        {
            FBXSDK_printf("Importer failed to load the file!\n\n");
        }

        if (lImporter->GetStatus() != FbxStatus::eSuccess)
            FBXSDK_printf("   Last error message: %s\n", lImporter->GetStatus().GetErrorString());

        FbxArray<FbxString*> history;
        lImporter->GetStatus().GetErrorStringHistory(history);
        if (history.GetCount() > 1)
        {
            FBXSDK_printf("   Error history stack:\n");
            for (int i = 0; i < history.GetCount(); i++)
            {
                FBXSDK_printf("      %s\n", history[i]->Buffer());
            }
        }
        FbxArrayDelete<FbxString*>(history);
        FBXSDK_printf("********************************************************************************\n");
    }

    // Destroy the importer.
    lImporter->Destroy();

    return lStatus;
}

// Helper
int FindBoneIndexByName(FbxString boneName) {
    for (int i = 0; i < g_vecBones.size(); i++)
        if (boneName == g_vecBones[i].name)
            return i;

    return 0;
}
bool IsSkeletonNode(FbxNode* node) {
    int nodeAttributeCount = node->GetNodeAttributeCount();
    for (int i = 0; i < nodeAttributeCount; ++i) {
        FbxNodeAttribute* nodeAttribute = node->GetNodeAttributeByIndex(i);
        if (FbxNodeAttribute::eSkeleton == nodeAttribute->GetAttributeType())
            return true;
    }
    return false;
}
void GetGlobalMtx(const FbxMatrix m, float* fOut, int& offset) {
    FbxVector4 t, sh, s;
    FbxQuaternion r;
    double d;
    m.GetElements(t, r, sh, s, d);

    int i = 0;
    fOut[offset + i++] = r.mData[0];
    fOut[offset + i++] = r.mData[1];
    fOut[offset + i++] = r.mData[2];
    fOut[offset + i++] = r.mData[3];
    fOut[offset + i++] = t.mData[0];
    fOut[offset + i++] = t.mData[1];
    fOut[offset + i++] = t.mData[2];
    offset += i;
}
void GetKeyTransform(const FbxAMatrix m, const FbxTime time, float* fOut, int& offset) {
    FbxQuaternion r = m.GetQ();
    FbxVector4 t = m.GetT();

    int i = 0;
    fOut[offset + i++] = time.GetSecondDouble();
    fOut[offset + i++] = r.mData[0];
    fOut[offset + i++] = r.mData[1];
    fOut[offset + i++] = r.mData[2];
    fOut[offset + i++] = r.mData[3];
    fOut[offset + i++] = t.mData[0];
    fOut[offset + i++] = t.mData[1];
    fOut[offset + i++] = t.mData[2];
    offset += i;
}

// Animation
void GetBoneHierarchyRec(FbxNode* node, int myIdx, int parentIdx) {
    if (node->GetNodeAttribute() &&
        node->GetNodeAttribute()->GetAttributeType() &&
        node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {

        Bone bone;
        bone.name = node->GetName();
        bone.node = node;
        bone.parentIdx = parentIdx;
        g_vecBones.push_back(bone);
    }
    for (unsigned int i = 0; i < node->GetChildCount(); ++i)
        GetBoneHierarchyRec(node->GetChild(i), g_vecBones.size(), myIdx);
}
void GetBoneHierarchy(FbxNode* rootNode) {
    for (int i = 0; i < rootNode->GetChildCount(); i++)
        GetBoneHierarchyRec(rootNode->GetChild(i), 0, -1);
}

void GetBoneBindpose(FbxScene* scene) {
    int nPose = scene->GetPoseCount();

    for (int iPose = 0; iPose < nPose; iPose++) {
        FbxPose* pose = scene->GetPose(iPose);
        if (!pose->IsBindPose()) continue;
        for (int i = 0; i < pose->GetCount(); i++) {
            FbxString name = pose->GetNodeName(i).GetCurrentName();
            for (auto iter = g_vecBones.begin(); iter != g_vecBones.end(); iter++) {
                if (iter->name != name) continue;
                iter->globalMatrix = pose->GetMatrix(i);
            }
        }
    }
}

void GetKeytimeFromCurve(FbxAnimCurve* curve, int idx) {
    for (int i = 0; i < curve->KeyGetCount(); ++i) {
        FbxTime keytime = curve->KeyGetTime(i);
        g_vecBones[idx].keytimes.insert(keytime);
    }
}
void retrieveAnimNode(FbxNode* node, FbxAnimLayer* layer) {
    FbxAnimCurve* curve;

    if (IsSkeletonNode(node)) {
        std::cout << node->GetName() << "\n";
        FbxString name = node->GetName();
        int idx = FindBoneIndexByName(name);

        curve = node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
        if (curve) GetKeytimeFromCurve(curve, idx);

        curve = node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Y);
        if (curve) GetKeytimeFromCurve(curve, idx);

        curve = node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Z);
        if (curve) GetKeytimeFromCurve(curve, idx);

        curve = node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
        if (curve) GetKeytimeFromCurve(curve, idx);

        curve = node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Y);
        if (curve) GetKeytimeFromCurve(curve, idx);

        curve = node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Z);
        if (curve) GetKeytimeFromCurve(curve, idx);
    }

    for (int i = 0; i < node->GetChildCount(); ++i)
        retrieveAnimNode(node->GetChild(i), layer);
}
void GetKeytime(FbxScene* scene) {
    for (int i = 0; i < scene->GetSrcObjectCount<FbxAnimStack>(); ++i)
    {
        FbxAnimStack* stack = scene->GetSrcObject<FbxAnimStack>(i);
        for (int j = 0; j < stack->GetMemberCount<FbxAnimLayer>(); ++j) {
            FbxAnimLayer* layer = stack->GetMember<FbxAnimLayer>(j);

            retrieveAnimNode(scene->GetRootNode(), layer);
        }
    }
}

bool IsRootBone(Bone bone) { return (bone.parentIdx == -1); }
FbxAMatrix GetTransfromEachTime(Bone bone, FbxTime time) {
    if (IsRootBone(bone)) {
        return bone.node->EvaluateLocalTransform(time).Inverse();
    }
    else {
        return bone.node->EvaluateLocalTransform(time).Inverse() * GetTransfromEachTime(g_vecBones[bone.parentIdx], time);
    }
}

void GetBoneEachTimeTransfrom() {
    for (int i = 0; i < g_vecBones.size(); i++) {
        for (auto iter = g_vecBones[i].keytimes.begin(); iter != g_vecBones[i].keytimes.end(); iter++) {
            g_vecBones[i].transforms.push_back(GetTransfromEachTime(g_vecBones[i], *iter));
        }
    }
}



















void GetAnimationData(FbxScene* scene) {
    GetBoneHierarchy(scene->GetRootNode()); // 1. 각 Bone별 구조를 만든다.
    GetBoneBindpose(scene);                 // 2. 각 Bone별 Binepose를 구한다.
    GetKeytime(scene);                      // 3. 각 Bone별 Keytime을 구한다.
    GetBoneEachTimeTransfrom();             // 4. 각 Bone의 Keytime별 Transfrom을 구한다.
}

void ExportAnimation(const char* fileName) {
    if (g_vecBones.empty()) { cout << "Animation Data not exist.\n"; return; }

    string ultimateOfPerfectFilePath;
    string fileHead = "Output/";
    string fileTail = ".mac";   // my Anim Clip

    ultimateOfPerfectFilePath = fileHead + fileName;
    ultimateOfPerfectFilePath += fileTail;

    ofstream out;
    out.open(ultimateOfPerfectFilePath, ios::out | ios::binary);

    int nBones;
    int* nKeys;

    nBones = g_vecBones.size();
    nKeys = new int[nBones];
    for (int i = 0; i < nBones; i++) nKeys[i] = g_vecBones[i].keytimes.size();

    out.write((char*)&nBones, sizeof(int));
    out.write((char*)&nKeys, sizeof(int) * nBones);

    for (int iBones = 0; iBones < nBones; iBones++) {
        int nTotalFloat = 7 + 8 * nKeys[iBones];
        float* pDest = new float[nTotalFloat];
        vector<FbxTime> keytimes;
        for (auto iter = g_vecBones[iBones].keytimes.begin(); iter != g_vecBones[iBones].keytimes.end(); iter++) keytimes.push_back(*iter);

        int i = 0;
        GetGlobalMtx(g_vecBones[iBones].globalMatrix, pDest, i);
        for (int iKeys = 0; iKeys < nKeys[iBones]; iKeys++) GetKeyTransform(g_vecBones[iBones].transforms[iKeys], keytimes[iKeys], pDest, i);

        out.write((char*)&pDest, sizeof(float) * nTotalFloat);
    }

    out.close();
}


void main() {

    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult;

    InitializeSdkObjects(lSdkManager, lScene);

    string fileName("animTest2_2");
    string fileHead("Input/");
    string fileTail(".FBX");
    string filePath;
    filePath = fileHead + fileName;
    filePath += fileTail;

    lResult = LoadScene(lSdkManager, lScene, filePath.c_str());

    GetAnimationData(lScene);


    ExportAnimation(fileName.c_str());
}

