// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialDirectionalLightEntity.h"
#include "PICOSpatialContainer.h"

void FDirectionalLightEntityProxy::ConstructDirectionalLightEntityData(USceneComponent* SceneComponent, bool bInitialize)
{
	if (const ULightComponent* LightComponent = Cast<ULightComponent>(SceneComponent))
	{
		if (bInitialize)
		{
			((LightComponent->Mobility != EComponentMobility::Static) && LightComponent->CastShadows && LightComponent->CastDynamicShadows) ? bCastShadow = true : bCastShadow = false;

			Lux = LightComponent->Intensity * 1000 / 2.0;
			LightColor = ToSpColor(LightComponent->LightColor);
		}
	}

	if (ParentEntityProxy && (ParentEntityProxy->bTransformDirty || bTransformDirty) && ParentEntityProxy->Owner.IsValid())
	{
		if (APICOSpatialContainer* Container = Cast<APICOSpatialContainer>(ParentEntityProxy->Owner.Get()->GetOwner()))
		{
			Transform.SetLocation(Container->DirectionalLightSocket->GetComponentLocation());
			bTransformDirty = true;
		}
	}
}

FDirectionalLightEntityProxy::FDirectionalLightEntityProxy(FSceneEntityProxy* ParentEntity, USceneComponent* SceneComponent)
	:FSceneEntityProxy(ParentEntity, SceneComponent)
{
	ConstructDirectionalLightEntityData(SceneComponent, true);
}

void FDirectionalLightEntityProxy::UpdateEntityProxy(USceneComponent* SceneComponent)
{
	FSceneEntityProxy::UpdateEntityProxy(SceneComponent);
	ConstructDirectionalLightEntityData(SceneComponent, false);
}

bool FDirectionalLightEntityProxy::InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer)
{
	if (FSceneEntityProxy::InitializeNativeEntity(InCurrentScene, SpatialContainer))
	{
		if (NativeEntity.Get()->IsValid())
		{
#if PLATFORM_ANDROID
			if (InCurrentScene)
			{
				DirectionalLightOffsetEntity = InCurrentScene->createEntity();
				DirectionalLightOffsetEntity.getOrAddTransformComponent().setRotation(ToSpQuat(FRotator(0, 90, 0).Quaternion()));
				DirectionalLightOffsetEntity.setParent(NativeEntity.Get()->entity);
				DirectionalLightOffsetEntity.setName(TCHAR_TO_UTF8( * (Name + TEXT("_Offset"))));
				DirectionalLightOffsetEntity.getOrAddDirectionalLightComponent().setColor(LightColor);
				DirectionalLightOffsetEntity.getOrAddDirectionalLightComponent().setIntensity(Lux);
				DirectionalLightOffsetEntity.getOrAddDirectionalLightComponent().setCastsShadowEnabled(bCastShadow);
				DirectionalLightOffsetEntity.getOrAddDirectionalLightComponent().setShadowProjectionTypeFixed(0.01f, 1.5f, 1.5f, 1.5f);
			}
#endif // PLATFORM_ANDROID
			return true;
		}
	}
	return false;
}

void FDirectionalLightEntityProxy::UpdateNativeEntity()
{
	FSceneEntityProxy::UpdateNativeEntity();
}
