// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialDynamicMeshEntity.h"
#include "PICOSpatial.h"
#include "SpatialPack.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "GeometryScript/ListUtilityFunctions.h"

using float3 = spatialpack::Vector3f;

void FDynamicMeshEntityProxy::ConstructDynamicMeshEntityData(USceneComponent* SceneComponent, bool bInitialize)
{
	if (UDynamicMeshComponent* DynamicMeshComponent = Cast<UDynamicMeshComponent>(SceneComponent))
	{
		DynamicMesh = DynamicMeshComponent->GetDynamicMesh();
		if (bInitialize)
		{
			bNeedRebuild = true;
		}
	}
}

FDynamicMeshEntityProxy::FDynamicMeshEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent)
	:FMeshEntityProxy(InParentEntityProxy, SceneComponent)
{
	ConstructDynamicMeshEntityData(SceneComponent, true);
	check(DynamicMesh != nullptr);
	DelegateHandle = DynamicMesh->OnMeshChanged().AddRaw(this, &FDynamicMeshEntityProxy::OnDynamicMeshModified);
}

FDynamicMeshEntityProxy::~FDynamicMeshEntityProxy()
{
	if (DynamicMesh)
	{
		DynamicMesh->OnMeshChanged().Remove(DelegateHandle);
		DelegateHandle.Reset();
	}
}

void FDynamicMeshEntityProxy::UpdateEntityProxy(USceneComponent* SceneComponent)
{
	FMeshEntityProxy::UpdateEntityProxy(SceneComponent);
	ConstructDynamicMeshEntityData(SceneComponent, false);
}

bool FDynamicMeshEntityProxy::InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer)
{
	if (FMeshEntityProxy::InitializeNativeEntity(InCurrentScene, SpatialContainer))
	{
		if (NativeEntity.Get()->IsValid())
		{
			return true;
		}
	}
	return false;
}

void FDynamicMeshEntityProxy::UpdateNativeEntity()
{
	FMeshEntityProxy::UpdateNativeEntity();
	RebuildDynamicMesh();
}

void FDynamicMeshEntityProxy::OnDynamicMeshModified(UDynamicMesh* InDynamicMesh, FDynamicMeshChangeInfo ChangeInfo)
{
	if (InDynamicMesh && DynamicMesh && InDynamicMesh->GetUniqueID() == DynamicMesh->GetUniqueID())
	{
		bNeedRebuild = true;
	}
}

void FDynamicMeshEntityProxy::RebuildDynamicMesh()
{
	if (!NativeEntity.Get()->IsValid() || !bNeedRebuild)
	{
		return;
	}

	TArray<float3> Positions;
	TArray<uint32> Indices;
	TArray<spatialpack::Vector2f> UV0;
	TArray<float3> Normals;

	FGeometryScriptIndexList IndexList{};
	bool bHasGap = false;
	UGeometryScriptLibrary_MeshQueryFunctions::GetAllTriangleIDs(DynamicMesh, IndexList, bHasGap);
	int32 TriCount = UGeometryScriptLibrary_ListUtilityFunctions::GetIndexListLength(IndexList);

	Positions.Reserve(TriCount * 3);
	Indices.Reserve(TriCount * 3);
	UV0.Reserve(TriCount * 3);
	Normals.Reserve(TriCount * 3);
	for (int32 TriangleID = 0; TriangleID < TriCount; TriangleID++)
	{
		FVector V1{}, V2{}, V3{};
		bool bValidIndex = false;
		bool bValidTriangle = false;
		int32 TriangleIndex = UGeometryScriptLibrary_ListUtilityFunctions::GetIndexListItem(IndexList, TriangleID, bValidIndex);
		UGeometryScriptLibrary_MeshQueryFunctions::GetTrianglePositions(DynamicMesh, TriangleIndex, bValidTriangle, V1, V2, V3);
		Positions.Add(ToSpVector(V1, 100.0f));
		Positions.Add(ToSpVector(V2, 100.0f));
		Positions.Add(ToSpVector(V3, 100.0f));

		FVector2D UV1{}, UV2{}, UV3{};
		bool bValidUV = false;
		UGeometryScriptLibrary_MeshQueryFunctions::GetTriangleUVs(DynamicMesh, 0, TriangleIndex, UV1, UV2, UV3, bValidUV);
		UV0.Add({ (float)UV1.X,(float)-UV1.Y });
		UV0.Add({ (float)UV2.X,(float)-UV2.Y });
		UV0.Add({ (float)UV3.X,(float)-UV3.Y });

		for (int32 i = 0; i < 3; i++)
		{
			Indices.Add(TriangleID * 3 + i);
			Normals.Add(ToSpVector(UGeometryScriptLibrary_MeshQueryFunctions::GetTriangleFaceNormal(DynamicMesh, TriangleIndex, bValidTriangle)));
		}
	}

#if PLATFORM_ANDROID
	if (TriCount > 0)
	{
		spatialpack::MeshModel MeshModel = spatialpack::MeshModelBuilder()
			.setPositions(Positions.GetData(), Positions.Num())
			.setTriangles(Indices.GetData(), Indices.Num())
			.setUV0(UV0.GetData(), UV0.Num())
			.setNormals(Normals.GetData(), Normals.Num())
			.allMaterialIndex(0)
			.build();

		spatialpack::MeshResource Mesh = spatialpack::MeshResource::createWithMeshModel(MoveTemp(MeshModel));
		NativeEntity.Get()->entity.getOrAddModelComponent().setMesh(Mesh);
	}
	else
	{
		spatialpack::MeshResource Mesh;
		NativeEntity.Get()->entity.getOrAddModelComponent().setMesh(Mesh);
	}
#endif
	bNeedRebuild = false;
}
