// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialInstancedStaticMeshEntity.h"
#include "PICOSpatial.h"

void FInstancedStaticMeshEntityProxy::ConstructInstancedStaticMeshEntityData(USceneComponent* SceneComponent, bool bInitialize)
{
	if (UInstancedStaticMeshComponent* InstancedStaticMeshComponent = Cast<UInstancedStaticMeshComponent>(SceneComponent))
	{
		if(bInitialize)
		{
			bNeedReConstructInstances = true;
		}

		bRenderInstancesDirty = InstancedStaticMeshComponent->IsRenderInstancesDirty() || bNeedReConstructInstances;

		if (InstancedStaticMeshComponent->PerInstanceSMData.Num() < 1)
		{
			if (StaticMesh != "")
			{
				StaticMesh = "";
				bMeshChanged = true;
			}
		}
	}
}

FInstancedStaticMeshEntityProxy::FInstancedStaticMeshEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent)
	:FStaticMeshEntityProxy(InParentEntityProxy, SceneComponent)
{
	ConstructInstancedStaticMeshEntityData(SceneComponent, true);
}

void FInstancedStaticMeshEntityProxy::UpdateEntityProxy(USceneComponent* SceneComponent)
{
	FStaticMeshEntityProxy::UpdateEntityProxy(SceneComponent);
	ConstructInstancedStaticMeshEntityData(SceneComponent, false);
}

bool FInstancedStaticMeshEntityProxy::InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer)
{
	if (FStaticMeshEntityProxy::InitializeNativeEntity(InCurrentScene, SpatialContainer))
	{
#if PLATFORM_ANDROID
		if (NativeEntity.Get()->IsValid())
		{
			MeshInstancesResource = spatialpack::MeshInstancesResource::create();
			auto model = NativeEntity.Get()->entity.getOrAddModelComponent();
			model.setMeshInstances(MeshInstancesResource);
		}
#endif // PLATFORM_ANDROID
		return true;
	}
	return false;
}

void FInstancedStaticMeshEntityProxy::UpdateNativeEntity()
{
	FStaticMeshEntityProxy::UpdateNativeEntity();
#if PLATFORM_ANDROID
	if (bRenderInstancesDirty)
	{
		UInstancedStaticMeshComponent* InstancedStaticMeshComponent = Cast<UInstancedStaticMeshComponent>(Owner.Get());
		if (!InstancedStaticMeshComponent)
		{
			return;
		}

		MeshInstancesResource.removeAllInstance();

		for (int32 i = 0; i < InstancedStaticMeshComponent->PerInstanceSMData.Num(); i++)
		{
			MeshInstancesResource.addInstance(TCHAR_TO_UTF8(*FString::FromInt(i)), ToSpMatrix(InstancedStaticMeshComponent->PerInstanceSMData[i].Transform, 100.0f));
		}
	
		MeshInstancesResource.removeInstance("Default");
		bRenderInstancesDirty = bNeedReConstructInstances = false;
	}
#endif // PLATFORM_ANDROID
}
