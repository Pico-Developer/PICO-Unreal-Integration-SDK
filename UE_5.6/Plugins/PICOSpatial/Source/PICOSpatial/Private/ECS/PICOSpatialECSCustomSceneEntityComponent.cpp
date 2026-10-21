#include "ECS/PICOSpatialECSCustomSceneEntityComponent.h"

#include "Components/ArrowComponent.h"
#include "ECS/PICOSpatialECSEntity.h"
#include "EntityProxy/PICOSpatialCustomSceneEntity.h"

UCustomSceneEntityComponent::UCustomSceneEntityComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	OrientationArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("OrientationArrow"));
	if (OrientationArrow)
	{
		OrientationArrow->SetupAttachment(this);
#if WITH_EDITORONLY_DATA
		OrientationArrow->SetIsVisualizationComponent(true);
#endif
		OrientationArrow->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
		OrientationArrow->SetHiddenInGame(true);
		OrientationArrow->ArrowColor = FColor(0, 200, 255);
		OrientationArrow->ArrowSize = 1.5f;
		OrientationArrow->bIsScreenSizeScaled = true;
	}
}

bool UCustomSceneEntityComponent::HasECSEntity() const
{
	return ECSEntity != nullptr;
}

UPICOSpatialECSEntity* UCustomSceneEntityComponent::GetOrCreateECSEntity()
{
	if (!ECSEntity)
	{
		ECSEntity = NewObject<UPICOSpatialECSEntity>(this, UPICOSpatialECSEntity::StaticClass(), NAME_None, RF_Transactional);
	}

#if PLATFORM_ANDROID
	if (CustomSceneEntityProxy)
	{
		CustomSceneEntityProxy->RefreshNativeEntityFromComponent();
	}
#endif
	return ECSEntity;
}

void UCustomSceneEntityComponent::ClearECSEntity()
{
#if PLATFORM_ANDROID
	if (ECSEntity)
	{
		ECSEntity->DestroyNativeEntity();
	}
#endif
	ECSEntity = nullptr;
}

#if PLATFORM_ANDROID
void UCustomSceneEntityComponent::SetCustomSceneEntityProxy(FCustomSceneEntityProxy* InProxy)
{
	CustomSceneEntityProxy = InProxy;
}
#endif
