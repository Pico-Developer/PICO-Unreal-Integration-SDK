// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialSceneEntity.h"
#include "PICOSpatialRuntimeSettings.h"
#include "PICOSpatial.h"
#include "PICOSpatialContainer.h"
#include "Components/ArrowComponent.h"
#include "PICOSpatialFunctionLibrary.h"

FNativeEntity::~FNativeEntity()
{
#if PLATFORM_ANDROID
	if (entity.isValid())
	{
		UE_LOG(LogPICOSpatial, Verbose, TEXT("DestroyNativeEntity:%hs, uuid:%llu"), entity.getName(), (uint64_t)entity.getEntityUUID());
		entity.destroy();
	}
#endif // PLATFORM_ANDROID
}

bool FNativeEntity::IsValid()
{
#if PLATFORM_ANDROID
	return entity.isValid();
#else
	return false;
#endif // PLATFORM_ANDROID
}

FPICOSpatial& FSceneEntityProxy::GetCachedPICOSpatial()
{
	if (UNLIKELY(CachedPICOSpatialPtr == nullptr))
	{
		CachedPICOSpatialPtr = &FPICOSpatial::Get();
	}
	return *CachedPICOSpatialPtr;
}

void FSceneEntityProxy::ConstructSceneEntityData(USceneComponent* SceneComponent, bool bInitialize)
{
	if (bInitialize)
	{
		Type = GetEntityProxyType(SceneComponent);
		Owner = SceneComponent;
		if (SceneComponent != nullptr)
		{
			const AActor* OwnerActor = nullptr;
			if ((OwnerActor = SceneComponent->GetOwner()) != nullptr)
			{
				if (SceneComponent == OwnerActor->GetRootComponent())
				{
#if WITH_EDITOR
					Name = OwnerActor->GetActorLabel();
#else
					Name = OwnerActor->GetActorNameOrLabel();
#endif
				}
				else
				{
					Name = SceneComponent->GetName();
				}
			}
			else
			{
				Name = "";
			}

			StoredTransform = Transform = SceneComponent->GetComponentTransform();
			bTransformDirty = true;
		}
	}
	else if(SceneComponent != nullptr)
	{
		const FTransform& NewTransform = SceneComponent->GetComponentTransform();
		bTransformDirty = bTransformDirty || (SceneComponent->Mobility == EComponentMobility::Movable ? !StoredTransform.Equals(NewTransform) : false);

		if (bTransformDirty)
		{
			StoredTransform = Transform = NewTransform;
		}
	}

	if (SceneComponent)
	{
		bVisible = SceneComponent->IsVisible();
	}
}

FSceneEntityProxy::FSceneEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent)
	:bIsSpatialContainerRootEntity(false)
{
	NativeEntity = MakeShared<FNativeEntity>();
	ParentEntityProxy = InParentEntityProxy;
	if (InParentEntityProxy != nullptr)
	{
		ParentEntity = InParentEntityProxy->NativeEntity.Get();
	}
	else
	{
		ParentEntity = nullptr;
	}

	ConstructSceneEntityData(SceneComponent, true);

	UE_LOG(LogPICOSpatial, Verbose, TEXT("CreateEntityProxy Type:%d, Name:%s"), Type, *Name);
}

FSceneEntityProxy::~FSceneEntityProxy()
{
#if PLATFORM_ANDROID
	if (NativeEntity.Get()->IsValid())
	{
		GetCachedPICOSpatial().NativeEntityToSceneComponentMap.Remove(NativeEntity.Get()->entity.getEntityUUID());
		NativeEntity.Reset();
	}
#endif // PLATFORM_ANDROID

	// Notify blueprints that this component is no longer rendered by the PICO Spatial system.
	// Owner may already be GC'd here, so pass nullptr in that case - callers must handle null.
	if (bHasBroadcastProxyCreated)
	{
		USceneComponent* OwnerComponent = Owner.Get();
		UPICOSpatialFunctionLibrary::GetEventManagerPICO()->OnComponentProxyDestroyed.Broadcast(OwnerComponent);
	}
}

void FSceneEntityProxy::UpdateEntityProxy(USceneComponent* SceneComponent)
{
	ConstructSceneEntityData(SceneComponent, false);
}

bool FSceneEntityProxy::InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer)
{
	if (InCurrentScene == nullptr)
	{
		return false;
	}
#if PLATFORM_ANDROID

	if (!NativeEntity.Get()->IsValid())
	{
		NativeEntity.Get()->entity = InCurrentScene->createEntity();
		NativeEntity.Get()->entity.setName(TCHAR_TO_UTF8(*Name));
		UE_LOG(LogPICOSpatial, Verbose, TEXT("Created new native entity:%s"), *Name);
	}

	if (NativeEntity.Get()->IsValid())
	{
		if (SpatialContainer)
		{
			SpatialContainer->addEntity(NativeEntity.Get()->entity);
		}
		else
		{
			UE_LOG(LogPICOSpatial, Warning, TEXT("Failed to add new native entity:%s to spatial container!"), *Name);
		}
	
		GetCachedPICOSpatial().NativeEntityToSceneComponentMap.Add(NativeEntity.Get()->entity.getEntityUUID(), this);

		if (ParentEntity == nullptr)
		{
		}
		else
		{
			if (ParentEntity->entity.isValid())
			{
				NativeEntity.Get()->entity.setParent(ParentEntity->entity);
			}
		}
	}

	const bool bSuccess = NativeEntity.Get()->IsValid();
	return bSuccess;
#endif // PLATFORM_ANDROID
	return false;
}

void FSceneEntityProxy::NotifyProxyCreatedIfNeeded()
{
	if (!bHasBroadcastProxyCreated)
	{
		bHasBroadcastProxyCreated = true;
		UPICOSpatialFunctionLibrary::GetEventManagerPICO()->OnComponentProxyCreated.Broadcast(Owner.Get());
	}
}

void FSceneEntityProxy::UpdateNativeEntity()
{
#if PLATFORM_ANDROID
	FNativeEntity* Entity = NativeEntity.Get();
	if (Entity && Entity->entity.isValid())
	{
		if (bTransformDirty)
		{
			Entity->entity.getOrAddTransformComponent().setPosition(ToSpVector(Transform.GetLocation(), 100.0f));
			Entity->entity.getOrAddTransformComponent().setRotation(ToSpQuat(Transform.GetRotation()));
			Entity->entity.getOrAddTransformComponent().setScale(ToSpVector(Transform.GetScale3D()));
			UE_LOG(LogPICOSpatial, Verbose, TEXT("Set Component:%s Transform:%s"), *Name, *(Transform.ToString()));
			bTransformDirty = false;
		}
	}
#endif // PLATFORM_ANDROID
}
