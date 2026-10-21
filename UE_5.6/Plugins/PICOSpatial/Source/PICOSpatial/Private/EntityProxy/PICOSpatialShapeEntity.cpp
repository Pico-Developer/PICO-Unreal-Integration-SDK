// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialShapeEntity.h"

void FShapeEntityProxy::ConstructShapeEntityData(USceneComponent* SceneComponent, bool bInitialize)
{
}

FShapeEntityProxy::FShapeEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent)
	:FPrimitiveEntityProxy(InParentEntityProxy, SceneComponent)
{
}

void FShapeEntityProxy::UpdateEntityProxy(USceneComponent* SceneComponent)
{
	FPrimitiveEntityProxy::UpdateEntityProxy(SceneComponent);
}

bool FShapeEntityProxy::InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer)
{
	if (FPrimitiveEntityProxy::InitializeNativeEntity(InCurrentScene, SpatialContainer))
	{
		if (NativeEntity.Get()->IsValid())
		{
			return true;
		}
	}
	return false;
}

void FShapeEntityProxy::UpdateNativeEntity()
{
	FPrimitiveEntityProxy::UpdateNativeEntity();
}
