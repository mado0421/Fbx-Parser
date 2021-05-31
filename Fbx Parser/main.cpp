#include "stdafx.h"
#include "Animation.h"
#include "Mesh.h"


void main() {

    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult;

    string fileName("pistol");
    string fileHead("Input/");
    string fileTail(".FBX");
    string filePath;
    filePath = fileHead + fileName;
    filePath += fileTail;

    InitializeSdkObjects(lSdkManager, lScene);

    lResult = LoadScene(lSdkManager, lScene, filePath.c_str());

    FbxAxisSystem d3dAxisSystem(FbxAxisSystem::EUpVector::eYAxis, FbxAxisSystem::EFrontVector::eParityOdd, FbxAxisSystem::ECoordSystem::eLeftHanded);   //FbxAxisSystem::eDirectX
    d3dAxisSystem.DeepConvertScene(lScene);


    // Animation
    GetAnimationData(lScene);
    //ExportAnimation(fileName.c_str());


    // Mesh
    ProcessSkeletonHierarchy(lScene->GetRootNode(), lScene);
    GetMeshData(lScene->GetRootNode());
    ExportModel(fileName.c_str());
}

