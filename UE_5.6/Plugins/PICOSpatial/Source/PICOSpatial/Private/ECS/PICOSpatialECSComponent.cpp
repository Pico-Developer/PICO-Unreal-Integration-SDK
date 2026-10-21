#include "ECS/PICOSpatialECSComponent.h"

bool UPICOSpatialECSComponent::InitializeNativeComponent(const spatialpack::SceneEntity& InNativeEntity)
{
#if PLATFORM_ANDROID
	if (!InNativeEntity.isValid())
	{
		bHasCachedNativeEntity = false;
		CachedNativeEntity = spatialpack::SceneEntity();
		return false;
	}

	CachedNativeEntity = InNativeEntity;
	bHasCachedNativeEntity = CachedNativeEntity.isValid();
	OnInitializeNativeComponent(CachedNativeEntity);
	if (!bAddedAtRuntime)
	{
		ApplyAllPropertiesToNative();
	}
	bHasCachedNativeEntity = CachedNativeEntity.isValid();
	return bHasCachedNativeEntity;
#else
	return false;
#endif
}

bool UPICOSpatialECSComponent::HasNativeEntity() const
{
#if PLATFORM_ANDROID
	return bHasCachedNativeEntity && CachedNativeEntity.isValid();
#else
	return false;
#endif
}

bool UPICOSpatialECSComponent::IsAddedAtRuntime() const
{
	return bAddedAtRuntime;
}

void UPICOSpatialECSComponent::MarkAddedAtRuntime()
{
	bAddedAtRuntime = true;
}
