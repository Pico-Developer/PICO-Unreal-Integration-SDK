#include "ECS/PICOSpatialECSEntity.h"
#include "ECS/PICOSpatialECSComponent.h"
#include "ECS/PICOSpatialECSTransformComponent.h"
#include "ECS/PICOSpatialECSTextComponent.h"
#include "ECS/PICOSpatialECSVideoPlayerComponent.h"
#include "ECS/PICOSpatialECSModelComponent.h"
#include "ECS/PICOSpatialECSGaussianSplattingComponent.h"
#include "PICOSpatialContainer.h"
#include "PICOSpatial.h"
#include "PICOSpatialSceneEntity.h"

#if PLATFORM_ANDROID
static void DetachComponentFromNativeEntity(UPICOSpatialECSComponent* Component, spatialpack::SceneEntity& NativeEntity)
{
	if (Component == nullptr)
	{
		return;
	}

	switch (Component->GetComponentType())
	{
	case EPICOSpatialECSComponentType::Transform:
		NativeEntity.removeTransformComponent();
		break;
	case EPICOSpatialECSComponentType::Text:
		NativeEntity.removeTextComponent();
		break;
	case EPICOSpatialECSComponentType::VideoPlayer:
		if (UPICOSpatialECSVideoPlayerComponent* VideoPlayer = Cast<UPICOSpatialECSVideoPlayerComponent>(Component))
		{
			VideoPlayer->ReleaseNativePlayer();
		}
		NativeEntity.removeVideoPlayerComponent();
		break;
	case EPICOSpatialECSComponentType::Model:
		NativeEntity.removeModelComponent();
		break;
	case EPICOSpatialECSComponentType::GaussianSplatting:
		if (UPICOSpatialECSGaussianSplattingComponent* Gaussian = Cast<UPICOSpatialECSGaussianSplattingComponent>(Component))
		{
			Gaussian->ReleaseNativeResource();
		}
		NativeEntity.removeGaussianSplattingComponent();
		break;
	default:
		break;
	}

	Component->InitializeNativeComponent(spatialpack::SceneEntity());
}
#endif

UPICOSpatialECSComponent* UPICOSpatialECSEntity::GetComponentByType(EPICOSpatialECSComponentType Type) const
{
	if (Type == EPICOSpatialECSComponentType::Unknown)
	{
		return nullptr;
	}

	for (UPICOSpatialECSComponent* Comp : Components)
	{
		if (Comp && Comp->GetComponentType() == Type)
		{
			return Comp;
		}
	}

	return nullptr;
}

bool UPICOSpatialECSEntity::HasComponent(EPICOSpatialECSComponentType Type) const
{
	return GetComponentByType(Type) != nullptr;
}

UPICOSpatialECSComponent* UPICOSpatialECSEntity::AddComponent(TSubclassOf<UPICOSpatialECSComponent> ComponentClass)
{
	if (!ComponentClass)
	{
		return nullptr;
	}

	UPICOSpatialECSComponent* NewComp = NewObject<UPICOSpatialECSComponent>(this, ComponentClass);
	if (!NewComp)
	{
		return nullptr;
	}

	const EPICOSpatialECSComponentType NewType = NewComp->GetComponentType();
	if (NewType != EPICOSpatialECSComponentType::Unknown)
	{
		if (UPICOSpatialECSComponent* Existing = GetComponentByType(NewType))
		{
			NewComp->MarkAsGarbage();
			return Existing;
		}
	}

	NewComp->MarkAddedAtRuntime();
	Components.Add(NewComp);
	NewComp->Initialize();

#if PLATFORM_ANDROID
	if (NativeEntity.isValid())
	{
		NewComp->InitializeNativeComponent(NativeEntity);
	}
#endif

	return NewComp;
}

bool UPICOSpatialECSEntity::RemoveComponentByType(EPICOSpatialECSComponentType Type)
{
	for (int32 i = 0; i < Components.Num(); ++i)
	{
		UPICOSpatialECSComponent* Comp = Components[i];
		if (Comp && Comp->GetComponentType() == Type)
		{
#if PLATFORM_ANDROID
			if (NativeEntity.isValid())
			{
				DetachComponentFromNativeEntity(Comp, NativeEntity);
			}
			else
			{
				Comp->InitializeNativeComponent(spatialpack::SceneEntity());
			}
#endif
			Components.RemoveAt(i);
			return true;
		}
	}
	return false;
}

bool UPICOSpatialECSEntity::RemoveComponent(UPICOSpatialECSComponent* Component)
{
	if (Component == nullptr)
	{
		return false;
	}

	for (int32 i = 0; i < Components.Num(); ++i)
	{
		UPICOSpatialECSComponent* Comp = Components[i];
		if (Comp != Component)
		{
			continue;
		}

#if PLATFORM_ANDROID
		if (NativeEntity.isValid())
		{
			DetachComponentFromNativeEntity(Comp, NativeEntity);
		}
		else
		{
			Comp->InitializeNativeComponent(spatialpack::SceneEntity());
		}
#endif
		Components.RemoveAt(i);
		return true;
	}

	return false;
}

UPICOSpatialECSTextComponent* UPICOSpatialECSEntity::GetTextComponent() const
{
	return Cast<UPICOSpatialECSTextComponent>(GetComponentByType(EPICOSpatialECSComponentType::Text));
}

UPICOSpatialECSTextComponent* UPICOSpatialECSEntity::GetOrAddTextComponent()
{
	if (UPICOSpatialECSTextComponent* Existing = GetTextComponent())
	{
		return Existing;
	}

	return Cast<UPICOSpatialECSTextComponent>(AddComponent(UPICOSpatialECSTextComponent::StaticClass()));
}

UPICOSpatialECSVideoPlayerComponent* UPICOSpatialECSEntity::GetVideoPlayerComponent() const
{
	return Cast<UPICOSpatialECSVideoPlayerComponent>(GetComponentByType(EPICOSpatialECSComponentType::VideoPlayer));
}

UPICOSpatialECSVideoPlayerComponent* UPICOSpatialECSEntity::GetOrAddVideoPlayerComponent()
{
	if (UPICOSpatialECSVideoPlayerComponent* Existing = GetVideoPlayerComponent())
	{
		return Existing;
	}

	return Cast<UPICOSpatialECSVideoPlayerComponent>(AddComponent(UPICOSpatialECSVideoPlayerComponent::StaticClass()));
}

UPICOSpatialECSTransformComponent* UPICOSpatialECSEntity::GetTransformComponent() const
{
	return Cast<UPICOSpatialECSTransformComponent>(GetComponentByType(EPICOSpatialECSComponentType::Transform));
}

UPICOSpatialECSTransformComponent* UPICOSpatialECSEntity::GetOrAddTransformComponent()
{
	if (UPICOSpatialECSTransformComponent* Existing = GetTransformComponent())
	{
		return Existing;
	}

	UPICOSpatialECSTransformComponent* NewComp = NewObject<UPICOSpatialECSTransformComponent>(this, UPICOSpatialECSTransformComponent::StaticClass());
	Components.Add(NewComp);
#if PLATFORM_ANDROID
	if (NativeEntity.isValid())
	{
		NewComp->MarkAddedAtRuntime();
	}
#endif
	NewComp->Initialize();

#if PLATFORM_ANDROID
	if (NativeEntity.isValid())
	{
		NewComp->InitializeNativeComponent(NativeEntity);
	}
#endif

	return NewComp;
}

UPICOSpatialECSModelComponent* UPICOSpatialECSEntity::GetModelComponent() const
{
	return Cast<UPICOSpatialECSModelComponent>(GetComponentByType(EPICOSpatialECSComponentType::Model));
}

UPICOSpatialECSModelComponent* UPICOSpatialECSEntity::GetOrAddModelComponent()
{
	if (UPICOSpatialECSModelComponent* Existing = GetModelComponent())
	{
		return Existing;
	}

	return Cast<UPICOSpatialECSModelComponent>(AddComponent(UPICOSpatialECSModelComponent::StaticClass()));
}

UPICOSpatialECSGaussianSplattingComponent* UPICOSpatialECSEntity::GetGaussianSplattingComponent() const
{
	return Cast<UPICOSpatialECSGaussianSplattingComponent>(GetComponentByType(EPICOSpatialECSComponentType::GaussianSplatting));
}

UPICOSpatialECSGaussianSplattingComponent* UPICOSpatialECSEntity::GetOrAddGaussianSplattingComponent()
{
	if (UPICOSpatialECSGaussianSplattingComponent* Existing = GetGaussianSplattingComponent())
	{
		return Existing;
	}

	return Cast<UPICOSpatialECSGaussianSplattingComponent>(AddComponent(UPICOSpatialECSGaussianSplattingComponent::StaticClass()));
}

bool UPICOSpatialECSEntity::InitializeNativeEntity(APICOSpatialContainer* Container, FString NativeName)
{
#if PLATFORM_ANDROID
	FPICOSpatial& PICOSpatialInstance = FPICOSpatial::Get();
	if (PICOSpatialInstance.CurrentScene == nullptr)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS] InitializeNativeEntity failed: CurrentScene is null. Entity=%s"), *GetName());
		return false;
	}

        const bool bCreatedNativeEntityThisCall = !NativeEntity.isValid();
	if (!NativeEntity.isValid())
	{
		NativeEntity = PICOSpatialInstance.CurrentScene->createEntity();
		const FString ResolvedName = NativeName.IsEmpty() ? GetName() : NativeName;
		NativeEntity.setName(TCHAR_TO_UTF8(*ResolvedName));
	}
	
	NativeEntity.getOrAddTransformComponent();
	GetOrAddTransformComponent();

	spatialpack::AppForm* SpatialContainer = Container ? Container->SpatialContainer : nullptr;
	if (SpatialContainer)
	{
		if (!bNativeEntityAddedToContainer || LastSpatialContainer != SpatialContainer)
		{
			SpatialContainer->addEntity(NativeEntity);
			LastSpatialContainer = SpatialContainer;
			bNativeEntityAddedToContainer = true;
		}
	}
	else
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS] Container or SpatialContainer invalid when adding ECSEntity native. Entity=%s UUID=%llu"), *GetName(), (uint64)NativeEntity.getEntityUUID());
                if (bCreatedNativeEntityThisCall)
                {
                        DestroyNativeEntity();
                }
                return false;
	}

	if (!bNativeComponentsInitialized)
	{
		for (UPICOSpatialECSComponent* Comp : Components)
		{
			if (Comp)
			{
				Comp->InitializeNativeComponent(NativeEntity);
			}
		}
		bNativeComponentsInitialized = true;
	}

	UE_LOG(LogPICOSpatial, Log, TEXT("[ECS] ECSEntity native ready. Entity=%s UUID=%llu Components=%d HasText=%d HasModel=%d HasGaussian=%d"),
		*GetName(),
		(uint64)NativeEntity.getEntityUUID(),
		Components.Num(),
		GetTextComponent() != nullptr ? 1 : 0,
		GetModelComponent() != nullptr ? 1 : 0,
		GetGaussianSplattingComponent() != nullptr ? 1 : 0);

	return NativeEntity.isValid();
#else
	return false;
#endif
}

bool UPICOSpatialECSEntity::IsNativeEntityValid() const
{
#if PLATFORM_ANDROID
	return NativeEntity.isValid();
#else
	return false;
#endif
}

#if PLATFORM_ANDROID
bool UPICOSpatialECSEntity::InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer, const spatialpack::SceneEntity& ParentEntity, FString NativeName)
{
	if (InCurrentScene == nullptr)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS] InitializeNativeEntity(Scene,...) failed: Scene is null. Entity=%s"), *GetName());
		return false;
	}

        const bool bCreatedNativeEntityThisCall = !NativeEntity.isValid();
	if (!NativeEntity.isValid())
	{
		NativeEntity = InCurrentScene->createEntity();
		const FString ResolvedName = NativeName.IsEmpty() ? GetName() : NativeName;
		NativeEntity.setName(TCHAR_TO_UTF8(*ResolvedName));
	}
	
	NativeEntity.getOrAddTransformComponent();
	GetOrAddTransformComponent();

	if (ParentEntity.isValid())
	{
		NativeEntity.setParent(ParentEntity);
	}
	else
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS] ParentEntity invalid when initializing ECSEntity. Entity=%s ChildUUID=%llu"), *GetName(), (uint64)NativeEntity.getEntityUUID());
                if (bCreatedNativeEntityThisCall)
                {
                        DestroyNativeEntity();
                }
                return false;
	}

	if (SpatialContainer)
	{
		if (!bNativeEntityAddedToContainer || LastSpatialContainer != SpatialContainer)
		{
			SpatialContainer->addEntity(NativeEntity);
			LastSpatialContainer = SpatialContainer;
			bNativeEntityAddedToContainer = true;
		}
	}
	else
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS] SpatialContainer null when adding ECSEntity native. Entity=%s UUID=%llu"), *GetName(), (uint64)NativeEntity.getEntityUUID());
                if (bCreatedNativeEntityThisCall)
                {
                        DestroyNativeEntity();
                }
                return false;
	}

	if (!bNativeComponentsInitialized)
	{
		for (UPICOSpatialECSComponent* Comp : Components)
		{
			if (Comp)
			{
				Comp->InitializeNativeComponent(NativeEntity);
			}
		}
		bNativeComponentsInitialized = true;
	}

	UE_LOG(LogPICOSpatial, Log, TEXT("[ECS] ECSEntity native ready (child). Entity=%s ChildUUID=%llu ParentUUID=%llu Components=%d HasText=%d HasModel=%d HasGaussian=%d"),
		*GetName(),
		(uint64)NativeEntity.getEntityUUID(),
		ParentEntity.isValid() ? (uint64)ParentEntity.getEntityUUID() : 0ull,
		Components.Num(),
		GetTextComponent() != nullptr ? 1 : 0,
		GetModelComponent() != nullptr ? 1 : 0,
		GetGaussianSplattingComponent() != nullptr ? 1 : 0);

	return NativeEntity.isValid();
}
#endif

#if PLATFORM_ANDROID
void UPICOSpatialECSEntity::DestroyNativeEntity()
{
	if (UPICOSpatialECSVideoPlayerComponent* VideoPlayer = GetVideoPlayerComponent())
	{
		VideoPlayer->ReleaseNativePlayer();
	}

	if (UPICOSpatialECSGaussianSplattingComponent* Gaussian = GetGaussianSplattingComponent())
	{
		Gaussian->ReleaseNativeResource();
	}

	for (UPICOSpatialECSComponent* Comp : Components)
	{
		if (Comp)
		{
			Comp->InitializeNativeComponent(spatialpack::SceneEntity());
		}
	}

	if (NativeEntity.isValid())
	{
		UE_LOG(LogPICOSpatial, Log, TEXT("[ECS] Destroy ECSEntity native. Entity=%s UUID=%llu"), *GetName(), (uint64)NativeEntity.getEntityUUID());
		NativeEntity.destroy();
		NativeEntity = spatialpack::SceneEntity();
	}
	LastSpatialContainer = nullptr;
	bNativeEntityAddedToContainer = false;
	bNativeComponentsInitialized = false;
}
#endif

void UPICOSpatialECSEntity::BeginDestroy()
{
#if PLATFORM_ANDROID
	DestroyNativeEntity();
#endif

	Super::BeginDestroy();
}

void UPICOSpatialECSEntity::PostLoad()
{
	Super::PostLoad();
#if WITH_EDITOR
	ValidateComponents();
#endif
}

#if WITH_EDITOR
void UPICOSpatialECSEntity::ValidateComponents()
{
	TSet<EPICOSpatialECSComponentType> SeenTypes;
	for (int32 i = Components.Num() - 1; i >= 0; --i)
	{
		UPICOSpatialECSComponent* Comp = Components[i];
		if (!Comp)
		{
			continue;
		}

		const EPICOSpatialECSComponentType Type = Comp->GetComponentType();
		if (Type == EPICOSpatialECSComponentType::Unknown)
		{
			continue;
		}

		if (SeenTypes.Contains(Type))
		{
			UE_LOG(LogPICOSpatial, Warning,
				TEXT("UPICOSpatialECSEntity: Duplicate component of type '%s' found and removed during validation."),
				*UEnum::GetValueAsString(Type));
			Components.RemoveAt(i);
		}
		else
		{
			SeenTypes.Add(Type);
		}
	}
}

void UPICOSpatialECSEntity::PostEditImport()
{
	Super::PostEditImport();
	ValidateComponents();
}

void UPICOSpatialECSEntity::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	static const FName ComponentsPropertyName = GET_MEMBER_NAME_CHECKED(UPICOSpatialECSEntity, Components);
	if (PropertyChangedEvent.GetPropertyName() != ComponentsPropertyName)
	{
		return;
	}

	// Only validate when a value is assigned to a slot
	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::ValueSet)
	{
		return;
	}

	// Find the index of the slot that was just assigned
	const int32 ChangedIndex = PropertyChangedEvent.GetArrayIndex(ComponentsPropertyName.ToString());
	if (!Components.IsValidIndex(ChangedIndex))
	{
		return;
	}

	UPICOSpatialECSComponent* ChangedComp = Components[ChangedIndex];
	if (!ChangedComp)
	{
		return;
	}

	const EPICOSpatialECSComponentType ChangedType = ChangedComp->GetComponentType();
	if (ChangedType == EPICOSpatialECSComponentType::Unknown)
	{
		return;
	}

	// Check if the same type already exists in another slot
	for (int32 i = 0; i < Components.Num(); ++i)
	{
		if (i == ChangedIndex)
		{
			continue;
		}

		UPICOSpatialECSComponent* Comp = Components[i];
		if (!Comp)
		{
			continue;
		}

		if (Comp->GetComponentType() == ChangedType)
		{
			UE_LOG(LogPICOSpatial, Warning,
				TEXT("UPICOSpatialECSEntity: A component of type '%s' already exists. This assignment has been reverted."),
				*UEnum::GetValueAsString(ChangedType));

			// Revert the slot back to null instead of removing it
			Components[ChangedIndex] = nullptr;
			break;
		}
	}
}
#endif
