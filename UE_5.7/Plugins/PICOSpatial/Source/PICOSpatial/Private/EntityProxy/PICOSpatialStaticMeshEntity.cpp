// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialStaticMeshEntity.h"
#include "PICOSpatial.h"
#include "Engine/StaticMesh.h"
#include "UObject/Package.h"

void FStaticMeshEntityProxy::ConstrucStaticMeshEntityData(USceneComponent* SceneComponent, bool bInitialize)
{
	if (!bVisible)
	{
		return;
	}

	if (const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(SceneComponent))
	{
		FString NewMeshPath = "";
		if (UStaticMesh* Mesh = StaticMeshComponent->GetStaticMesh())
		{
			NewMeshPath = Mesh->GetPackage()->GetPathName();
		}

		bMeshChanged = NewMeshPath != StaticMesh;

		if (bMeshChanged)
		{
			StaticMesh = NewMeshPath;
		}
	}
}

FStaticMeshEntityProxy::FStaticMeshEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent)
	:FMeshEntityProxy(InParentEntityProxy, SceneComponent)
	, StaticMesh("")
	, bMeshChanged(false)
{
	ConstrucStaticMeshEntityData(SceneComponent, true);
}

void FStaticMeshEntityProxy::UpdateEntityProxy(USceneComponent* SceneComponent)
{
	FMeshEntityProxy::UpdateEntityProxy(SceneComponent);
	ConstrucStaticMeshEntityData(SceneComponent, false);
}

bool FStaticMeshEntityProxy::InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer)
{
	if (FMeshEntityProxy::InitializeNativeEntity(InCurrentScene, SpatialContainer))
	{
		if (NativeEntity.Get()->IsValid())
		{
			spatialpack::MeshResource* MeshResource = nullptr;
			MeshResource = FPICOSpatial::Get().CacheMeshResource(StaticMesh);
#if PLATFORM_ANDROID
			if (bCreateSimpleCollision)
			{
				if (Collision.ConvexElems.Num() > 0 && MeshResource && ( * MeshResource).isValid())
				{
					spatialpack::ShapeResource ShapeResource;
					ShapeResource = spatialpack::ShapeResource::createConvexMesh( * MeshResource);
					ShapeResources.Add(ShapeResource);
				}
			}
			if (bCreateComplexCollision)
			{
				if (MeshResource && ( * MeshResource).isValid())
				{
					spatialpack::ShapeResource ShapeResource;
					ShapeResource = spatialpack::ShapeResource::createStaticMesh( * MeshResource);
					ShapeResources.Add(ShapeResource);
				}
			}
#endif // PLATFORM_ANDROID
			return true;
		}
	}
	return false;
}

void FStaticMeshEntityProxy::UpdateNativeEntity()
{
	FMeshEntityProxy::UpdateNativeEntity();
#if PLATFORM_ANDROID
	spatialpack::SceneEntity entity = NativeEntity.Get()->entity;
	if (!entity.isValid())
	{
		return;
	}
	if (bMeshChanged)
	{
		if (StaticMesh != "")
		{
			spatialpack::MeshResource* Mesh = FPICOSpatial::Get().CacheMeshResource(StaticMesh);
			if (Mesh)
			{
				UE_LOG(LogPICOSpatial, Verbose, TEXT("Set StaticMeshComponent:%s, StaticMesh:%s, NativeMesh:%llu"), *Name, *StaticMesh, Mesh->getResourceId());
				entity.getOrAddModelComponent().setMesh( * Mesh);
			}
		}
		else
		{
			spatialpack::MeshResource Mesh;
			entity.getOrAddModelComponent().setMesh(Mesh);
		}
		bMeshChanged = false;
	}
#endif // PLATFORM_ANDROID
}
