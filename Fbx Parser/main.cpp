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

//#define REFLECT
#ifdef REFLECT

#else

#endif

using namespace DirectX;
using namespace std;

struct Bone {
    FbxString           name;
    int                 parentIdx;
    FbxNode*            node;

    XMFLOAT4X4 globalMatrix, toParent, toDressposeInv;
    vector<XMFLOAT4X4> toWorld;

};

vector<Bone> g_vecBones;
set<FbxTime> g_setKeytimes;


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
void GetKeyTransform(const FbxAMatrix& m, float* fOut, int& offset) {
    FbxVector4 t = m.GetT();
    FbxQuaternion r = m.GetQ();
    
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
void GetXMFLOAT4X4(const XMFLOAT4X4& m, float* fOut, int& offset) {
    XMMATRIX mm = XMLoadFloat4x4(&m);
    XMVECTOR r = XMQuaternionRotationMatrix(mm);

    int i = 0;
    fOut[offset + i++] = XMVectorGetX(r);
    fOut[offset + i++] = XMVectorGetY(r);
    fOut[offset + i++] = XMVectorGetZ(r);
    fOut[offset + i++] = XMVectorGetW(r);
    fOut[offset + i++] = m._41;
    fOut[offset + i++] = m._42;
    fOut[offset + i++] = m._43;
    offset += i;
}

XMFLOAT4X4 FbxAMtxConvertToXMFLOAT4X4(const FbxAMatrix& m) {
    XMFLOAT4X4 result;

    FbxVector4 t = m.GetT();
    FbxQuaternion r = m.GetQ();


    XMStoreFloat4x4(&result, XMMatrixMultiply(XMMatrixRotationQuaternion(XMVectorSet(r.mData[0], r.mData[1], r.mData[2], r.mData[3])), XMMatrixTranslation(t.mData[0], t.mData[1], t.mData[2])));

    return result;
}
XMFLOAT4X4 FbxMtxConvertToXMFLOAT4X4(const FbxMatrix& m) {
    XMFLOAT4X4 result;

    FbxVector4 t, sh, s;
    FbxQuaternion r;
    double d;
    m.GetElements(t, r, sh, s, d);

    XMStoreFloat4x4(&result, XMMatrixMultiply(XMMatrixRotationQuaternion(XMVectorSet(r.mData[0], r.mData[1], r.mData[2], r.mData[3])), XMMatrixTranslation(t.mData[0], t.mData[1], t.mData[2])));

    return result;
}


float Eps(float f) {
    if (abs(f) <= 0.001f) return 0;
    return f;
}
XMFLOAT3 ConvertToFloat3(const FbxQuaternion q) {
    float x = q.mData[0];
    float y = q.mData[1];
    float z = q.mData[2];
    float w = q.mData[3];
    float sqw = w * w;
    float sqx = x * x;
    float sqy = y * y;
    float sqz = z * z;
    XMFLOAT3 result;
    result.x = asinf(2.0f * (w * x - y * z)); // rotation about x-axis
    result.y = atan2f(2.0f * (x * z + w * y), (-sqx - sqy + sqz + sqw)); // rotation about y-axis
    result.z = atan2f(2.0f * (x * y + w * z), (-sqx + sqy - sqz + sqw)); // rotation about z-axis
    return result;
}
XMFLOAT3 ConvertToFloat3(const XMFLOAT4& q) {
    float x = q.x;
    float y = q.y;
    float z = q.z;
    float w = q.w;
    float sqw = w * w;
    float sqx = x * x;
    float sqy = y * y;
    float sqz = z * z;
    XMFLOAT3 result;
    result.x = asinf(2.0f * (w * x - y * z)); // rotation about x-axis
    result.y = atan2f(2.0f * (x * z + w * y), (-sqx - sqy + sqz + sqw)); // rotation about y-axis
    result.z = atan2f(2.0f * (x * y + w * z), (-sqx + sqy - sqz + sqw)); // rotation about z-axis
    return result;
}
void PrintFbxAMatrix(const FbxAMatrix m) {
    FbxVector4 t = m.GetT();
    FbxQuaternion r = m.GetQ();
    XMFLOAT3 xmf3R = ConvertToFloat3(r);

    cout << "\t - T   : " << Eps(t.mData[0]) << ", " << Eps(t.mData[1]) << ", " << Eps(t.mData[2]) << "\n";
    cout << "\t - R(Q): " << Eps(r.mData[0]) << ", " << Eps(r.mData[1]) << ", " << Eps(r.mData[2]) << ", " << Eps(r.mData[3]) << "\n";
    cout << "\t - R(D): " << Eps(XMConvertToDegrees(xmf3R.x)) << ", " << Eps(XMConvertToDegrees(xmf3R.y)) << ", " << Eps(XMConvertToDegrees(xmf3R.z)) << "\n\n";
}
void PrintFbxMatrix(const FbxMatrix m) {
    FbxVector4 t, sh, s;
    FbxQuaternion r;
    double d;
    m.GetElements(t, r, sh, s, d);
    XMFLOAT3 xmf3R = ConvertToFloat3(r);

    cout << "\t - T   : " << Eps(t.mData[0]) << ", " << Eps(t.mData[1]) << ", " << Eps(t.mData[2]) << "\n";
    cout << "\t - R(Q): " << Eps(r.mData[0]) << ", " << Eps(r.mData[1]) << ", " << Eps(r.mData[2]) << ", " << Eps(r.mData[3]) << "\n";
    cout << "\t - R(D): " << Eps(XMConvertToDegrees(xmf3R.x)) << ", " << Eps(XMConvertToDegrees(xmf3R.y)) << ", " << Eps(XMConvertToDegrees(xmf3R.z)) << "\n";
    cout << "\t - S: " << Eps((s.mData[0])) << ", " << Eps((s.mData[1])) << ", " << Eps((s.mData[2])) << "\n";
}
void PrintMtx(const XMMATRIX& m) {
    XMFLOAT4 rotation, translation;
    XMFLOAT4X4 xmf4x4result;
    float yaw, pitch, roll;

    XMStoreFloat4(&rotation, XMQuaternionRotationMatrix(m));
    XMStoreFloat4x4(&xmf4x4result, m);
    XMFLOAT3 ypr = ConvertToFloat3(rotation);
    translation.x = xmf4x4result._41;
    translation.y = xmf4x4result._42;
    translation.z = xmf4x4result._43;

    cout
        << "=============================\n"
        << "- Rq: "
        << Eps(rotation.x) << ", "
        << Eps(rotation.y) << ", "
        << Eps(rotation.z) << ", "
        << Eps(rotation.w) << "\n"
        << "- Rv: "
        << Eps(XMConvertToDegrees(ypr.x)) << ", "
        << Eps(XMConvertToDegrees(ypr.y)) << ", "
        << Eps(XMConvertToDegrees(ypr.z)) << "\n"
        << "- Tv: "
        << Eps(translation.x) << ", "
        << Eps(translation.y) << ", "
        << Eps(translation.z) << "\n"
        << "=============================\n"
        << Eps(xmf4x4result._11) << "\t" << Eps(xmf4x4result._12) << "\t" << Eps(xmf4x4result._13) << "\t" << Eps(xmf4x4result._14) << "\n"
        << Eps(xmf4x4result._21) << "\t" << Eps(xmf4x4result._22) << "\t" << Eps(xmf4x4result._23) << "\t" << Eps(xmf4x4result._24) << "\n"
        << Eps(xmf4x4result._31) << "\t" << Eps(xmf4x4result._32) << "\t" << Eps(xmf4x4result._33) << "\t" << Eps(xmf4x4result._34) << "\n"
        << Eps(xmf4x4result._41) << "\t" << Eps(xmf4x4result._42) << "\t" << Eps(xmf4x4result._43) << "\t" << Eps(xmf4x4result._44) << "\n\n";
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
                iter->globalMatrix = FbxMtxConvertToXMFLOAT4X4(pose->GetMatrix(i));

#ifdef REFLECT
                XMStoreFloat4x4(&iter->globalMatrix, XMMatrixMultiply(XMLoadFloat4x4(&iter->globalMatrix), XMMatrixReflect(XMVectorSet(1, 0, 0, 0))));
#else

#endif

            }
        }
    }
}

void GetKeytimeFromCurve(FbxAnimCurve* curve) {
    for (int i = 0; i < curve->KeyGetCount(); ++i) {
        FbxTime keytime = curve->KeyGetTime(i);
        g_setKeytimes.insert(keytime);
    }
}
void retrieveAnimNode(FbxNode* node, FbxAnimLayer* layer) {
    FbxAnimCurve* curve;

    if (IsSkeletonNode(node)) {
        std::cout << node->GetName() << "\n";
        FbxString name = node->GetName();
        int idx = FindBoneIndexByName(name);

        curve = node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
        if (curve) GetKeytimeFromCurve(curve );

        curve = node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Y);
        if (curve) GetKeytimeFromCurve(curve );

        curve = node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Z);
        if (curve) GetKeytimeFromCurve(curve );

        curve = node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
        if (curve) GetKeytimeFromCurve(curve );

        curve = node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Y);
        if (curve) GetKeytimeFromCurve(curve );

        curve = node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Z);
        if (curve) GetKeytimeFromCurve(curve );
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

XMMATRIX GetTransfromEachTime(Bone bone, FbxTime time) {
    XMMATRIX result;
    XMFLOAT4X4 temp;
	temp = FbxAMtxConvertToXMFLOAT4X4(bone.node->EvaluateLocalTransform(time));
#ifdef REFLECT
    XMStoreFloat4x4(&temp, XMMatrixMultiply(XMLoadFloat4x4(&temp), XMMatrixReflect(XMVectorSet(1, 0, 0, 0))));

#else

#endif
    if (IsRootBone(bone)) 
        return XMLoadFloat4x4(&temp);
    else 
        return XMMatrixMultiply(  XMLoadFloat4x4(&temp), GetTransfromEachTime(g_vecBones[bone.parentIdx], time));
}

void GetBoneEachTimeTransfrom() {
    for (int i = 0; i < g_vecBones.size(); i++) {
        for (auto iter = g_setKeytimes.begin(); iter != g_setKeytimes.end(); iter++) {

            /*=================================================================
            여기서 toDressposeInv가 front
            EachTime이 back
            XMVECTOR p = XMVector3Transform(p0, XMMatrixMultiply(XMLoadFloat4x4(&g_vecBones[i].toDressposeInv), GetTransfromEachTime(g_vecBones[i], *iter)));
            이렇게 하면 Z축 반전된 상태로 애니메이션이 됨

            메쉬가 +Y방향으로 솟아있을 때
            맥스 애니메이션은 +Z방향으로 꺾어지는 모양이지만
            여기선 -Z방향으로 꺾어지고 있으므로
            
            1. 메쉬를 일단 +Y방향으로 돌린다
            2. Z방향 반전을 하든 아니면 이게 전후 반전이 아니라 Y축 기준 180도 회전일 수도 있음
            (상황에 따라서 X축 반전을 해주던가 뭔가 하여간~)
            =================================================================*/

            XMFLOAT4X4 toWorld;
            XMStoreFloat4x4(&toWorld, GetTransfromEachTime(g_vecBones[i], *iter));
            g_vecBones[i].toWorld.push_back(toWorld);
        }
    }
}

void GetToParentMtx() {

}

void GetToRootMtx() {

}

XMMATRIX GetToDressposeMtx(const Bone& b) {
    XMMATRIX result;
    if (IsRootBone(b))  return XMLoadFloat4x4(&b.toParent);
    else                return XMMatrixMultiply(XMLoadFloat4x4(&b.toParent), GetToDressposeMtx(g_vecBones[b.parentIdx]));
}

void GetBoneToParentAndToDressposeInvMtx() {
    for (int i = 0; i < g_vecBones.size(); i++) {
        XMMATRIX result;
        XMVECTOR det;
        
        if (g_vecBones[i].parentIdx == -1) {
            result = XMLoadFloat4x4(&g_vecBones[i].globalMatrix);
        }
        else {
            det = XMMatrixDeterminant(XMLoadFloat4x4(&g_vecBones[g_vecBones[i].parentIdx].globalMatrix));
            result = XMMatrixMultiply(XMLoadFloat4x4(&g_vecBones[i].globalMatrix), XMMatrixInverse(&det, XMLoadFloat4x4(&g_vecBones[g_vecBones[i].parentIdx].globalMatrix))  );
        }
        XMStoreFloat4x4(&g_vecBones[i].toParent, result);
        det = XMMatrixDeterminant(GetToDressposeMtx(g_vecBones[i]));
        XMStoreFloat4x4(&g_vecBones[i].toDressposeInv, XMMatrixInverse(&det, GetToDressposeMtx(g_vecBones[i])));

        cout 
            << "================================================\n"
            << "BoneIdx: " << i << "\n"
            << "================================================\n"
            << "BoneName: " << g_vecBones[i].name << "\n";
    }
}

void GetAnimationData(FbxScene* scene) {
    GetBoneHierarchy(scene->GetRootNode());   // 1. 각 Bone별 구조를 만든다.
    GetBoneBindpose(scene);                   // 2. 각 Bone별 Binepose를 구한다.
    GetBoneToParentAndToDressposeInvMtx();
    GetKeytime(scene);                        // 3. 각 Bone별 Keytime을 구한다.
    GetBoneEachTimeTransfrom();               // 4. 각 Bone의 Keytime별 Transfrom을 구한다.
}

void ReflectYZPlane() {
    for (auto it = g_vecBones.begin(); it != g_vecBones.end(); it++) {
        //XMStoreFloat4x4(&it->toDressposeInv, XMMatrixMultiply(XMLoadFloat4x4(&it->toDressposeInv), XMMatrixReflect(XMVectorSet(1, 0, 0, 0))));
        
        for (auto it2 = it->toWorld.begin(); it2 != it->toWorld.end(); it2++) 
            XMStoreFloat4x4(&(*it2), XMMatrixMultiply(XMLoadFloat4x4(&(*it2)), XMMatrixReflect(XMVectorSet(1, 0, 0, 0))));
    }
}
void ReflectXYPlane() {
    for (auto it = g_vecBones.begin(); it != g_vecBones.end(); it++) {
        //XMStoreFloat4x4(&it->toDressposeInv, XMMatrixMultiply(XMLoadFloat4x4(&it->toDressposeInv), XMMatrixReflect(XMVectorSet(0, 0, 1, 0))));

        for (auto it2 = it->toWorld.begin(); it2 != it->toWorld.end(); it2++)
            XMStoreFloat4x4(&(*it2), XMMatrixMultiply(XMLoadFloat4x4(&(*it2)), XMMatrixReflect(XMVectorSet(0, 0, 1, 0))));
    }
}

void ExportAnimation(const char* fileName) {
    if (g_vecBones.empty()) { cout << "Animation Data not exist.\n"; return; }


    string ultimateOfPerfectFilePath;
    string fileHead("../../../WindowsProject1/WindowsProject1/Assets/");
    //string fileHead = "Output/";
    string fileTail = ".mac";   // my Anim Clip

    ultimateOfPerfectFilePath = fileHead + fileName;
    ultimateOfPerfectFilePath += fileTail;

    ofstream out;
    out.open(ultimateOfPerfectFilePath, ios::out | ios::binary);

    int nBones;
    int nKeys;
    double* pKeytimes;

    nBones = g_vecBones.size();
    nKeys = g_setKeytimes.size();
    out.write((char*)&nBones, sizeof(int));
    out.write((char*)&nKeys, sizeof(int));

    pKeytimes = new double(nKeys);
    int i = 0;
    for (auto iter = g_setKeytimes.begin(); iter != g_setKeytimes.end(); iter++) 
        pKeytimes[i++] = iter->GetSecondDouble();
    
    out.write((char*)pKeytimes, sizeof(double) * nKeys);

    for (int iBones = 0; iBones < nBones; iBones++) {
        int nTotalFloat = 7 * (nKeys + 1);
        float* pDest = new float[nTotalFloat];

        int idx = 0;
        GetXMFLOAT4X4(g_vecBones[iBones].toDressposeInv, pDest, idx);
        for (int iKeys = 0; iKeys < nKeys; iKeys++) 
            GetXMFLOAT4X4(g_vecBones[iBones].toWorld[iKeys], pDest, idx);

        out.write((char*)pDest, sizeof(float) * nTotalFloat);
    }

    out.close();
}

void main() {

    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult;

    string fileName("Humanoid_Aiming");
    string fileHead("Input/");
    string fileTail(".FBX");
    string filePath;
    filePath = fileHead + fileName;
    filePath += fileTail;

    InitializeSdkObjects(lSdkManager, lScene);

    lResult = LoadScene(lSdkManager, lScene, filePath.c_str());

    FbxAxisSystem d3dAxisSystem(FbxAxisSystem::EUpVector::eYAxis, FbxAxisSystem::EFrontVector::eParityOdd, FbxAxisSystem::ECoordSystem::eLeftHanded);   //FbxAxisSystem::eDirectX
    d3dAxisSystem.DeepConvertScene(lScene);

    //FbxAxisSystem fbxAxisSystem = lScene->GetGlobalSettings().GetAxisSystem();
    //FbxAxisSystem d3dAxisSystem(FbxAxisSystem::eDirectX);
    //FbxAxisSystem d3dAxisSystem(FbxAxisSystem::EUpVector::eXAxis, FbxAxisSystem::EFrontVector::eParityEven, FbxAxisSystem::ECoordSystem::eLeftHanded);
    //FbxAxisSystem d3dAxisSystem(FbxAxisSystem::EUpVector::eXAxis, FbxAxisSystem::EFrontVector::eParityOdd, FbxAxisSystem::ECoordSystem::eLeftHanded);
    //FbxAxisSystem d3dAxisSystem(FbxAxisSystem::EUpVector::eXAxis, FbxAxisSystem::EFrontVector::eParityEven, FbxAxisSystem::ECoordSystem::eRightHanded);
    //FbxAxisSystem d3dAxisSystem(FbxAxisSystem::EUpVector::eXAxis, FbxAxisSystem::EFrontVector::eParityOdd, FbxAxisSystem::ECoordSystem::eRightHanded);
    //FbxAxisSystem d3dAxisSystem(FbxAxisSystem::EUpVector::eYAxis, FbxAxisSystem::EFrontVector::eParityEven, FbxAxisSystem::ECoordSystem::eLeftHanded);
    //FbxAxisSystem d3dAxisSystem(FbxAxisSystem::EUpVector::eYAxis, FbxAxisSystem::EFrontVector::eParityEven, FbxAxisSystem::ECoordSystem::eRightHanded);
    //FbxAxisSystem d3dAxisSystem(FbxAxisSystem::EUpVector::eYAxis, FbxAxisSystem::EFrontVector::eParityOdd, FbxAxisSystem::ECoordSystem::eRightHanded);  ////YZ 전환 temp.m_xmf3Pos.z *= -1; 일 때, 좌우반전
    //FbxAxisSystem d3dAxisSystem(FbxAxisSystem::EUpVector::eZAxis, FbxAxisSystem::EFrontVector::eParityEven, FbxAxisSystem::ECoordSystem::eLeftHanded);
    //FbxAxisSystem d3dAxisSystem(FbxAxisSystem::EUpVector::eZAxis, FbxAxisSystem::EFrontVector::eParityOdd, FbxAxisSystem::ECoordSystem::eLeftHanded);
    //FbxAxisSystem d3dAxisSystem(FbxAxisSystem::EUpVector::eZAxis, FbxAxisSystem::EFrontVector::eParityEven, FbxAxisSystem::ECoordSystem::eRightHanded);
    //FbxAxisSystem d3dAxisSystem(FbxAxisSystem::EUpVector::eZAxis, FbxAxisSystem::EFrontVector::eParityOdd, FbxAxisSystem::ECoordSystem::eRightHanded);

    GetAnimationData(lScene);
    ExportAnimation(fileName.c_str());
}

