#pragma once
#include "stdafx.h"

// Mesh
struct CtrlPoint {
    XMFLOAT3    position;
    vector<pair<int, double>> boneInfo;
};
struct Vertex {
    UINT ctrlPointIndex;
    XMFLOAT3 normal;
    XMFLOAT3 binormal;
    XMFLOAT3 tangent;
    XMFLOAT2 uv;
};
struct Mesh {
    vector<CtrlPoint> vecCtrlPoint;
    vector<Vertex> vecVertex;
};
struct Joint {
    FbxString name;
    FbxNode* node;
    int parentIdx;
};
struct TransformForExport {
    float KeyTimeRotationTranslation[8];
};
struct BoneForExport {
    int nKeyframe;
    int parentIdx;
    float toParent[7];
    float toDressInv[7];
};
struct CtrlPointForExport {
    XMFLOAT3        position;
    unsigned int    boneIndices[4];
    double          weights[4];

    CtrlPointForExport()
        : position(XMFLOAT3(0, 0, 0))
    {
        memset(boneIndices, 0, sizeof(unsigned int) * 4);
        memset(weights, 0, sizeof(double) * 4);
    }
};
struct VertexForExport {
    UINT ctrlPointIndex;
    XMFLOAT3 normal;
    XMFLOAT3 binormal;
    XMFLOAT3 tangent;
    XMFLOAT2 uv;

    VertexForExport()
        : ctrlPointIndex(0)
        , normal(XMFLOAT3(0, 0, 0))
        , binormal(XMFLOAT3(0, 0, 0))
        , tangent(XMFLOAT3(0, 0, 0))
        , uv(XMFLOAT2(0, 0))
    {}
};

vector<Joint> vecJoint;
vector<TransformForExport> AnimationClip;
vector<Mesh> vecMesh;

unsigned int FindJointIndexByName(FbxString jointName) {
    for (unsigned int i = 0; i < vecJoint.size(); ++i) {
        if (jointName == vecJoint[i].name)
            return i;
    }
    return 0;
}
FbxAMatrix GetGeometryTransform(FbxNode* node) {
    const FbxVector4 lT = node->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 lR = node->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 lS = node->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(lT, lR, lS);
}

/*===============================================================
* Mesh
===============================================================*/
bool IsMeshNode(FbxNode* node) {
    int nodeAttributeCount = node->GetNodeAttributeCount();
    for (int i = 0; i < nodeAttributeCount; ++i) {
        FbxNodeAttribute* nodeAttribute = node->GetNodeAttributeByIndex(i);
        if (FbxNodeAttribute::eMesh == nodeAttribute->GetAttributeType()) return true;
    }

    return false;
}

void ReadNormal(FbxMesh* mesh, UINT ctrlPointIdx, UINT inVertexCounter, XMFLOAT3& outNormal) {
    /*===============================================================
    * 나는 FbxGeometryElement::eByPolygonVertex 를 사용할 것이므로
    * 그 외에는 따로 처리해줄 필요가 없다. (아마?)
    ===============================================================*/
    if (!mesh->GetElementNormalCount()) return;
    FbxGeometryElementNormal* vertexNormal = mesh->GetElementNormal();
    unsigned int index;

    switch (vertexNormal->GetMappingMode())
    {
    case FbxGeometryElement::eByPolygonVertex:
        switch (vertexNormal->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
            outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
            outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
            outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
            break;

        case FbxGeometryElement::eIndexToDirect:
            index = vertexNormal->GetIndexArray().GetAt(inVertexCounter);
            outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
            outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
            outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
            break;

        default: assert(!"ReadNormal, vertexNormal->GetReferenceMode() error."); break;
        }
        break;
    case FbxGeometryElement::eByControlPoint:
        switch (vertexNormal->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
            outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(ctrlPointIdx).mData[0]);
            outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(ctrlPointIdx).mData[1]);
            outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(ctrlPointIdx).mData[2]);
            break;
        case FbxGeometryElement::eIndexToDirect:
        {
            index = vertexNormal->GetIndexArray().GetAt(ctrlPointIdx);
            outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
            outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
            outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
            break;
        }
        default:
            throw std::exception("Invalid Reference");
        }
        break;

    default: assert(!"ReadNormal, vertexNormal->GetMappingMode() error."); break;
    }
}
void ReadBinormal(FbxMesh* mesh, UINT ctrlPointIdx, UINT inVertexCounter, XMFLOAT3& outBinormal) {

    if (!mesh->GetElementBinormalCount()) return;
    FbxGeometryElementBinormal* vertexBinormal = mesh->GetElementBinormal();
    unsigned int index;

    switch (vertexBinormal->GetMappingMode())
    {
    case FbxGeometryElement::eByPolygonVertex:
        switch (vertexBinormal->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
            outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
            outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
            outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
            break;

        case FbxGeometryElement::eIndexToDirect:
            index = vertexBinormal->GetIndexArray().GetAt(inVertexCounter);
            outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
            outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
            outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
            break;

        default: assert(!"ReadBinormal, vertexBinormal->GetReferenceMode() error."); break;
        }
        break;
    case FbxGeometryElement::eByControlPoint:
        switch (vertexBinormal->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
            outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(ctrlPointIdx).mData[0]);
            outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(ctrlPointIdx).mData[1]);
            outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(ctrlPointIdx).mData[2]);
            break;
        case FbxGeometryElement::eIndexToDirect:
        {
            index = vertexBinormal->GetIndexArray().GetAt(ctrlPointIdx);
            outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
            outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
            outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
            break;
        }
        default:
            throw std::exception("Invalid Reference");
        }
        break;

    default: assert(!"ReadBinormal, vertexBinormal->GetMappingMode() error."); break;
    }
}
void ReadTangent(FbxMesh* mesh, UINT ctrlPointIdx, UINT inVertexCounter, XMFLOAT3& outTangent) {
    if (!mesh->GetElementTangentCount()) return;
    FbxGeometryElementTangent* vertexTangent = mesh->GetElementTangent();
    unsigned int index;

    switch (vertexTangent->GetMappingMode())
    {
    case FbxGeometryElement::eByPolygonVertex:
        switch (vertexTangent->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
            outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[0]);
            outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[1]);
            outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[2]);
            break;

        case FbxGeometryElement::eIndexToDirect:
            index = vertexTangent->GetIndexArray().GetAt(inVertexCounter);
            outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
            outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
            outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
            break;

        default: assert(!"ReadTangent, vertexTangent->GetReferenceMode() error."); break;
        }
        break;
    case FbxGeometryElement::eByControlPoint:
        switch (vertexTangent->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
            outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(ctrlPointIdx).mData[0]);
            outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(ctrlPointIdx).mData[1]);
            outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(ctrlPointIdx).mData[2]);
            break;
        case FbxGeometryElement::eIndexToDirect:
        {
            index = vertexTangent->GetIndexArray().GetAt(ctrlPointIdx);
            outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
            outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
            outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
            break;
        }
        default:
            throw std::exception("Invalid Reference");
        }
        break;
    default: assert(!"ReadTangent, vertexTangent->GetMappingMode() error."); break;
    }
}
void ReadUV(FbxMesh* mesh, UINT ctrlPointIdx, UINT inVertexCounter, XMFLOAT2& outUV) {

    FbxGeometryElementUV* vertexUV = mesh->GetElementUV();
    unsigned int index;

    switch (vertexUV->GetMappingMode())
    {
    case FbxGeometryElement::eByPolygonVertex:
        switch (vertexUV->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
            outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inVertexCounter).mData[0]);
            outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inVertexCounter).mData[1]);
            break;

        case FbxGeometryElement::eIndexToDirect:
            index = vertexUV->GetIndexArray().GetAt(inVertexCounter);
            outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[0]);
            outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[1]);
            break;

        default: assert(!"ReadUV, vertexUV->GetReferenceMode() error."); break;
        }
        break;
    case FbxGeometryElement::eByControlPoint:
        switch (vertexUV->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
            outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(ctrlPointIdx).mData[0]);
            outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(ctrlPointIdx).mData[1]);
            break;
        case FbxGeometryElement::eIndexToDirect:
        {
            index = vertexUV->GetIndexArray().GetAt(ctrlPointIdx);
            outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[0]);
            outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[1]);
            break;
        }
        default:
            throw std::exception("Invalid Reference");
        }
        break;
    default: assert(!"ReadUV, vertexUV->GetMappingMode() error."); break;
    }
}

void LoadMesh(FbxNode* node) {
    if (!IsMeshNode(node)) return;
    FbxMesh* mesh = node->GetMesh();
    Mesh tempMesh;

    /*===============================================================
    * 1. Get CtrlPoint vector.
    *
    * here, we can get ctrlPoint position, not boneIndices and boneWeights.
    ===============================================================*/
    unsigned int nCtrlPointCount = mesh->GetControlPointsCount();
    FbxVector4* pCtrlPoints = mesh->GetControlPoints();
    for (unsigned int i = 0; i < nCtrlPointCount; ++i) {
        CtrlPoint tempCP;
        tempCP.position.x = pCtrlPoints[i].mData[0];
        tempCP.position.y = pCtrlPoints[i].mData[1];
        tempCP.position.z = pCtrlPoints[i].mData[2];
        tempMesh.vecCtrlPoint.push_back(tempCP);
    }

    /*===============================================================
    * 2. Get CtrlPoint boneInfo
    *
    ===============================================================*/
    FbxGeometry* geo = node->GetGeometry();
    UINT nDeformer = geo->GetDeformerCount(FbxDeformer::eSkin);
    for (UINT deformerIdx = 0; deformerIdx < nDeformer; ++deformerIdx) {
        FbxSkin* skin = reinterpret_cast<FbxSkin*>(geo->GetDeformer(deformerIdx, FbxDeformer::eSkin));
        if (!skin) continue;

        UINT nCluster = skin->GetClusterCount();
        for (UINT clusterIdx = 0; clusterIdx < nCluster; ++clusterIdx) {
            FbxCluster* cluster = skin->GetCluster(clusterIdx);
            int nIdx = cluster->GetControlPointIndicesCount();
            int* pIdx = cluster->GetControlPointIndices();
            double* pWeight = cluster->GetControlPointWeights();

            FbxString jointName = cluster->GetLink()->GetName();
            UINT jointIdx = FindJointIndexByName(jointName);

            for (UINT i = 0; i < nIdx; ++i) {
                pair<int, double> boneInfo;
                boneInfo.first = jointIdx;
                boneInfo.second = pWeight[i];
                tempMesh.vecCtrlPoint[pIdx[i]].boneInfo.push_back(boneInfo);
            }
        }
    }

    /*===============================================================
    * 3. Get Vertex vector.
    *
    ===============================================================*/
    UINT nPolygon = mesh->GetPolygonCount();
    UINT vertexCounter = 0;

    for (UINT i = 0; i < nPolygon; ++i) {
        UINT nPolygonSize = mesh->GetPolygonSize(i);
        for (UINT j = 0; j < nPolygonSize; ++j) {
            UINT ctrlPointIdx = mesh->GetPolygonVertex(i, j);
            XMFLOAT3 normal[3];
            XMFLOAT3 binormal[3];
            XMFLOAT3 tangent[3];
            XMFLOAT2 uv[3];
            ReadNormal(mesh, ctrlPointIdx, vertexCounter, normal[j]);
            ReadBinormal(mesh, ctrlPointIdx, vertexCounter, binormal[j]);
            ReadTangent(mesh, ctrlPointIdx, vertexCounter, tangent[j]);
            ReadUV(mesh, ctrlPointIdx, vertexCounter, uv[j]);
            Vertex tempVertex;
            tempVertex.ctrlPointIndex = ctrlPointIdx;
            tempVertex.normal = normal[j];
            tempVertex.binormal = binormal[j];
            tempVertex.tangent = tangent[j];
            tempVertex.uv = uv[j];
            tempMesh.vecVertex.push_back(tempVertex);
            vertexCounter++;
        }
    }



    vecMesh.push_back(tempMesh);
}
void GetMeshData(FbxNode* node) {
    LoadMesh(node);
    for (int i = 0; i < node->GetChildCount(); ++i) LoadMesh(node->GetChild(i));
    FbxNodeAttribute* nodeAttribute = node->GetNodeAttribute();
}

void PrintMesh() {
    for_each(vecMesh.begin(), vecMesh.end(), [](Mesh m) {
        cout << "nCtrlPoint: " << m.vecCtrlPoint.size() << "\n";

        cout << "CtrlPoint List:\n\n";
        for_each(m.vecCtrlPoint.begin(), m.vecCtrlPoint.end(), [](CtrlPoint cp) {
            cout << " - Position: " << cp.position.x << ", " << cp.position.y << ", " << cp.position.z << "\n";
            for_each(cp.boneInfo.begin(), cp.boneInfo.end(), [](pair<int, double> p) {
                cout << "\t - BoneIdx: " << p.first << ", weight: " << p.second << "\n"; });
            });
        cout << "\nCtrlPoint End\n\n";

        cout << "nVertex: " << m.vecVertex.size() << "\n";
        cout << "Vertex List:\n\n";
        for_each(m.vecVertex.begin(), m.vecVertex.end(), [](Vertex v) {
            cout << " - CtrlPointIdx: " << v.ctrlPointIndex << "\n";
            cout << " - Normal: " << v.normal.x << ", " << v.normal.y << ", " << v.normal.z << "\n";
            cout << " - Binormal: " << v.binormal.x << ", " << v.binormal.y << ", " << v.binormal.z << "\n";
            cout << " - Tangent: " << v.tangent.x << ", " << v.tangent.y << ", " << v.tangent.z << "\n";
            cout << " - UV: " << v.uv.x << ", " << v.uv.y << "\n\n";
            });
        cout << "\nVertex End\n\n";
        });
}

void ProcessSkeletonHierarchyRecursively(FbxNode* node, FbxScene* scene, unsigned int myIdx, unsigned int parentIdx) {
    if (node->GetNodeAttribute() &&
        node->GetNodeAttribute()->GetAttributeType() &&
        node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {

        Joint joint;
        joint.name = node->GetName();
        joint.node = node;
        joint.parentIdx = parentIdx;
        vecJoint.push_back(joint);

    }
    for (unsigned int i = 0; i < node->GetChildCount(); ++i)
        ProcessSkeletonHierarchyRecursively(node->GetChild(i), scene, vecJoint.size(), myIdx);
}
void ProcessSkeletonHierarchy(FbxNode* rootNode, FbxScene* scene) {
    for (unsigned int childIdx = 0; childIdx < rootNode->GetChildCount(); ++childIdx) {
        FbxNode* node = rootNode->GetChild(childIdx);
        ProcessSkeletonHierarchyRecursively(node, scene, 0, -1);
    }
    cout << "Skeleton List:\n\n";
    for_each(vecJoint.begin(), vecJoint.end(), [](Joint j) {
        cout << " - Name: " << j.name << "\n";
        });
    cout << "\nSkeleton End\n\n";
}

/*
    영향을 주는 Bone이 4개보다 많으면
    1. 가장 weight가 적은 Bone을 지우고
    2. 해당 weight를 나눠서 나머지 넷에게 준다
*/
void MakeNumOfBoneUnder4(CtrlPointForExport& cp, vector<pair<int, double>>& IdxWeight) {
    double dMin = FBXSDK_DOUBLE_MAX;
    if (IdxWeight.size() <= 4) {
        for (int i = 0; i < IdxWeight.size(); ++i) {
            cp.boneIndices[i] = IdxWeight[i].first;
            cp.weights[i] = IdxWeight[i].second;
        }
        return;
    }
    else {
        while (IdxWeight.size() > 4) {

            for (int i = 0; i < IdxWeight.size(); i++) {
                dMin = min(dMin, IdxWeight[i].second);
            }

            double weight;
            int count = 0;
            for (auto it = IdxWeight.begin(); it != IdxWeight.end();) {
                if (dMin == it->second) {
                    weight = it->second;
                    it = IdxWeight.erase(it);
                    count++;
                }
                else it++;
            }
            for (auto it = IdxWeight.begin(); it != IdxWeight.end(); it++) {
                it->second += (weight * count) / IdxWeight.size();
            }
            for (int i = 0; i < IdxWeight.size(); i++) {
                IdxWeight[i].second = max(1, IdxWeight[i].second);
                cp.boneIndices[i] = IdxWeight[i].first;
                cp.weights[i] = IdxWeight[i].second;
            }
        }
        return;
    }

}

void ExportModel(const char* fileName) {
    if (vecMesh.empty()) return;

    string ultimateOfPerfectFilePath;

    string fileHead = "../../../WindowsProject1/WindowsProject1/Assets/";
    string fileTail = ".mm";   // my mesh

    ultimateOfPerfectFilePath = fileHead + fileName;
    ultimateOfPerfectFilePath += fileTail;

    ofstream out;

    out.open(ultimateOfPerfectFilePath, ios::out | ios::binary);
    int nMesh = vecMesh.size();
    out.write((char*)&nMesh, sizeof(int));


    for (int iMesh = 0; iMesh < nMesh; iMesh++) {
        int nCtrlPoint = vecMesh[iMesh].vecCtrlPoint.size();
        out.write((char*)&nCtrlPoint, sizeof(int));

        CtrlPointForExport* pCtrlPoint = new CtrlPointForExport[nCtrlPoint];
        for (int iCP = 0; iCP < nCtrlPoint; ++iCP) {
            CtrlPointForExport temp2;
            temp2.position = vecMesh[iMesh].vecCtrlPoint[iCP].position;
            swap(temp2.position.y, temp2.position.z);
            temp2.position.z *= -1;

            MakeNumOfBoneUnder4(temp2, vecMesh[iMesh].vecCtrlPoint[iCP].boneInfo);
            pCtrlPoint[iCP] = temp2;
        }
        out.write((char*)pCtrlPoint, sizeof(CtrlPointForExport) * nCtrlPoint);

        int nVertex = vecMesh[iMesh].vecVertex.size();
        out.write((char*)&nVertex, sizeof(int));

        VertexForExport* pVertex = new VertexForExport[nVertex];
        for (int iV = 0; iV < nVertex; ++iV) {
            VertexForExport temp1;
            temp1.ctrlPointIndex    = vecMesh[iMesh].vecVertex[iV].ctrlPointIndex;
            temp1.normal            = vecMesh[iMesh].vecVertex[iV].normal;
            swap(temp1.normal.y, temp1.normal.z);
            temp1.normal.z *= -1;

            temp1.binormal          = vecMesh[iMesh].vecVertex[iV].binormal;


            temp1.tangent           = vecMesh[iMesh].vecVertex[iV].tangent;
            swap(temp1.tangent.y, temp1.tangent.z);
            temp1.tangent.z *= -1;

            temp1.uv                = vecMesh[iMesh].vecVertex[iV].uv;
            temp1.uv.y *= -1;


            pVertex[iV]             = temp1;
        }
        out.write((char*)pVertex, sizeof(VertexForExport) * nVertex);
    }

    out.close();
}
