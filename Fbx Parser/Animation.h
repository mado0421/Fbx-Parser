#pragma once
#include "stdafx.h"

// Anim
struct Bone {
    FbxString   name;
    FbxNode* node;
    int         parentIdx;

    XMFLOAT4X4 globalMatrix, toParent, toDressposeInv;
    vector<XMFLOAT4X4> toWorld;
};

inline void GetGlobalMtx(const FbxMatrix m, float* fOut, int& offset) {
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
inline void GetKeyTransform(const FbxAMatrix& m, float* fOut, int& offset) {
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
inline void GetXMFLOAT4X4(const XMFLOAT4X4& m, float* fOut, int& offset) {
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

inline XMFLOAT4X4 FbxAMtxConvertToXMFLOAT4X4(const FbxAMatrix& m) {
    XMFLOAT4X4 result;

    FbxVector4 t = m.GetT();
    FbxQuaternion r = m.GetQ();


    XMStoreFloat4x4(&result, XMMatrixMultiply(XMMatrixRotationQuaternion(XMVectorSet(r.mData[0], r.mData[1], r.mData[2], r.mData[3])), XMMatrixTranslation(t.mData[0], t.mData[1], t.mData[2])));

    return result;
}
inline XMFLOAT4X4 FbxMtxConvertToXMFLOAT4X4(const FbxMatrix& m) {
    XMFLOAT4X4 result;

    FbxVector4 t, sh, s;
    FbxQuaternion r;
    double d;
    m.GetElements(t, r, sh, s, d);

    XMStoreFloat4x4(&result, XMMatrixMultiply(XMMatrixRotationQuaternion(XMVectorSet(r.mData[0], r.mData[1], r.mData[2], r.mData[3])), XMMatrixTranslation(t.mData[0], t.mData[1], t.mData[2])));

    return result;
}

inline XMFLOAT3 ConvertToFloat3(const FbxQuaternion q) {
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
inline XMFLOAT3 ConvertToFloat3(const XMFLOAT4& q) {
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
//inline void PrintFbxAMatrix(const FbxAMatrix m) {
//    FbxVector4 t = m.GetT();
//    FbxQuaternion r = m.GetQ();
//    XMFLOAT3 xmf3R = ConvertToFloat3(r);
//    
//    cout << "\t - T   : " << Eps(t.mData[0]) << ", " << Eps(t.mData[1]) << ", " << Eps(t.mData[2]) << "\n";
//    cout << "\t - R(Q): " << Eps(r.mData[0]) << ", " << Eps(r.mData[1]) << ", " << Eps(r.mData[2]) << ", " << Eps(r.mData[3]) << "\n";
//    cout << "\t - R(D): " << Eps(XMConvertToDegrees(xmf3R.x)) << ", " << Eps(XMConvertToDegrees(xmf3R.y)) << ", " << Eps(XMConvertToDegrees(xmf3R.z)) << "\n\n";
//}
//inline void PrintFbxMatrix(const FbxMatrix m) {
//    FbxVector4 t, sh, s;
//    FbxQuaternion r;
//    double d;
//    m.GetElements(t, r, sh, s, d);
//    XMFLOAT3 xmf3R = ConvertToFloat3(r);
//
//    cout << "\t - T   : " << Eps(t.mData[0]) << ", " << Eps(t.mData[1]) << ", " << Eps(t.mData[2]) << "\n";
//    cout << "\t - R(Q): " << Eps(r.mData[0]) << ", " << Eps(r.mData[1]) << ", " << Eps(r.mData[2]) << ", " << Eps(r.mData[3]) << "\n";
//    cout << "\t - R(D): " << Eps(XMConvertToDegrees(xmf3R.x)) << ", " << Eps(XMConvertToDegrees(xmf3R.y)) << ", " << Eps(XMConvertToDegrees(xmf3R.z)) << "\n";
//    cout << "\t - S: " << Eps((s.mData[0])) << ", " << Eps((s.mData[1])) << ", " << Eps((s.mData[2])) << "\n";
//}
//inline void PrintMtx(const XMMATRIX& m) {
//    XMFLOAT4 rotation, translation;
//    XMFLOAT4X4 xmf4x4result;
//    float yaw, pitch, roll;
//
//    XMStoreFloat4(&rotation, XMQuaternionRotationMatrix(m));
//    XMStoreFloat4x4(&xmf4x4result, m);
//    XMFLOAT3 ypr = ConvertToFloat3(rotation);
//    translation.x = xmf4x4result._41;
//    translation.y = xmf4x4result._42;
//    translation.z = xmf4x4result._43;
//
//    cout
//        << "=============================\n"
//        << "- Rq: "
//        << Eps(rotation.x) << ", "
//        << Eps(rotation.y) << ", "
//        << Eps(rotation.z) << ", "
//        << Eps(rotation.w) << "\n"
//        << "- Rv: "
//        << Eps(XMConvertToDegrees(ypr.x)) << ", "
//        << Eps(XMConvertToDegrees(ypr.y)) << ", "
//        << Eps(XMConvertToDegrees(ypr.z)) << "\n"
//        << "- Tv: "
//        << Eps(translation.x) << ", "
//        << Eps(translation.y) << ", "
//        << Eps(translation.z) << "\n"
//        << "=============================\n"
//        << Eps(xmf4x4result._11) << "\t" << Eps(xmf4x4result._12) << "\t" << Eps(xmf4x4result._13) << "\t" << Eps(xmf4x4result._14) << "\n"
//        << Eps(xmf4x4result._21) << "\t" << Eps(xmf4x4result._22) << "\t" << Eps(xmf4x4result._23) << "\t" << Eps(xmf4x4result._24) << "\n"
//        << Eps(xmf4x4result._31) << "\t" << Eps(xmf4x4result._32) << "\t" << Eps(xmf4x4result._33) << "\t" << Eps(xmf4x4result._34) << "\n"
//        << Eps(xmf4x4result._41) << "\t" << Eps(xmf4x4result._42) << "\t" << Eps(xmf4x4result._43) << "\t" << Eps(xmf4x4result._44) << "\n\n";
//}
//


vector<Bone> g_vecBones;
set<FbxTime> g_setKeytimes;




// Helper
int FindBoneIndexByName(FbxString boneName) {
    for (int i = 0; i < g_vecBones.size(); i++)
        if (boneName == g_vecBones[i].name)
            return i;

    return 0;
}

bool IsRootBone(Bone bone) { return (bone.parentIdx == -1); }

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
        if (curve) GetKeytimeFromCurve(curve);

        curve = node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Y);
        if (curve) GetKeytimeFromCurve(curve);

        curve = node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Z);
        if (curve) GetKeytimeFromCurve(curve);

        curve = node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
        if (curve) GetKeytimeFromCurve(curve);

        curve = node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Y);
        if (curve) GetKeytimeFromCurve(curve);

        curve = node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Z);
        if (curve) GetKeytimeFromCurve(curve);
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
        return XMMatrixMultiply(XMLoadFloat4x4(&temp), GetTransfromEachTime(g_vecBones[bone.parentIdx], time));
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
            result = XMMatrixMultiply(XMLoadFloat4x4(&g_vecBones[i].globalMatrix), XMMatrixInverse(&det, XMLoadFloat4x4(&g_vecBones[g_vecBones[i].parentIdx].globalMatrix)));
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