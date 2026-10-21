#include "PICOSpatialCustomSceneEntity.h"

#include "ECS/PICOSpatialECSEntity.h"
#include "ECS/PICOSpatialECSCustomSceneEntityComponent.h"
#include "PICOSpatialFunctionLibrary.h"

FCustomSceneEntityProxy::FCustomSceneEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent)
	: FSceneEntityProxy(InParentEntityProxy, SceneComponent)
{
#if PLATFORM_ANDROID
	if (UCustomSceneEntityComponent* CustomComponent = Cast<UCustomSceneEntityComponent>(SceneComponent))
	{
		this->CustomComponent = CustomComponent;
		ECSEntity = CustomComponent->ECSEntity;
		CustomComponent->SetCustomSceneEntityProxy(this);
	}
#endif
}

FCustomSceneEntityProxy::~FCustomSceneEntityProxy()
{
#if PLATFORM_ANDROID
	if (UCustomSceneEntityComponent* PinnedComponent = CustomComponent.Get())
	{
		PinnedComponent->SetCustomSceneEntityProxy(nullptr);
		if (PinnedComponent->ECSEntity)
		{
			PinnedComponent->ECSEntity->DestroyNativeEntity();
			return;
		}
	}

	if (UPICOSpatialECSEntity* PinnedEntity = ECSEntity.Get())
	{
		PinnedEntity->DestroyNativeEntity();
	}
#endif
}

bool FCustomSceneEntityProxy::InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer)
{
        const bool bBaseResult = FSceneEntityProxy::InitializeNativeEntity(InCurrentScene, SpatialContainer);

#if PLATFORM_ANDROID
	LastInitializedScene = InCurrentScene;
	LastSpatialContainer = SpatialContainer;

	if (!bBaseResult)
	{
		return false;
	}

	const UCustomSceneEntityComponent* CustomComponent = Cast<UCustomSceneEntityComponent>(Owner.Get());
	if (!CustomComponent)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS] CustomSceneEntityProxy has no UCustomSceneEntityComponent owner. Owner=%s"), Owner.IsValid() ? *Owner->GetName() : TEXT("<null>"));
		return true;
	}

	UPICOSpatialECSEntity* ECSEntity = CustomComponent->ECSEntity.Get();
	if (!ECSEntity)
	{
		return true;
	}

	spatialpack::SceneEntity entity = NativeEntity.Get()->entity;
	if (!entity.isValid())
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS] CustomSceneEntityProxy native entity invalid. Owner=%s Entity=%s"), Owner.IsValid() ? *Owner->GetName() : TEXT("<null>"), *ECSEntity->GetName());
		return false;
	}

	const bool bChildResult = ECSEntity->InitializeNativeEntity(InCurrentScene, SpatialContainer, entity, ECSEntity->GetName());
	UE_LOG(LogPICOSpatial, Log, TEXT("[ECS] ECSEntity native initialized via proxy. Result=%d Owner=%s Entity=%s ParentUUID=%llu ChildUUID=%llu HasText=%d"),
		bChildResult ? 1 : 0,
		Owner.IsValid() ? *Owner->GetName() : TEXT("<null>"),
		*ECSEntity->GetName(),
		(uint64)entity.getEntityUUID(),
		bChildResult ? (uint64)ECSEntity->GetNativeEntity().getEntityUUID() : 0ull,
		ECSEntity->GetTextComponent() != nullptr ? 1 : 0);
	return bChildResult;
#endif

	return bBaseResult;
}

bool FCustomSceneEntityProxy::RefreshNativeEntityFromComponent()
{
#if PLATFORM_ANDROID
	if (LastInitializedScene == nullptr || !NativeEntity.IsValid() || !NativeEntity->IsValid())
	{
		return false;
	}

	UCustomSceneEntityComponent* CurrentComponent = Cast<UCustomSceneEntityComponent>(Owner.Get());
	if (CurrentComponent == nullptr || CurrentComponent->ECSEntity == nullptr)
	{
		return false;
	}

	UPICOSpatialECSEntity* CurrentEntity = CurrentComponent->ECSEntity.Get();
	if (CurrentEntity == nullptr)
	{
		return false;
	}

	if (UPICOSpatialECSEntity* PreviousEntity = ECSEntity.Get())
	{
		if (PreviousEntity != CurrentEntity && PreviousEntity->IsNativeEntityValid())
		{
			PreviousEntity->DestroyNativeEntity();
		}
	}

	ECSEntity = CurrentEntity;
	if (CurrentEntity->IsNativeEntityValid())
	{
		return true;
	}

	return CurrentEntity->InitializeNativeEntity(LastInitializedScene, LastSpatialContainer, NativeEntity->entity, CurrentEntity->GetName());
#else
	return false;
#endif
}
