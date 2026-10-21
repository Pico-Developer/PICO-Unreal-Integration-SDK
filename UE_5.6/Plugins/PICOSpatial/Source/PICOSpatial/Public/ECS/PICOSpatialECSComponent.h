#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ECS/PICOSpatialECSComponentType.h"
#include "PICOSpatialUtility.h"
#include "PICOSpatialECSComponent.generated.h"

class UPICOSpatialECSEntity;

UCLASS(Abstract, BlueprintType, EditInlineNew)
class PICOSPATIAL_API UPICOSpatialECSComponent : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Component")
	virtual void Initialize() {}

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Component")
	virtual EPICOSpatialECSComponentType GetComponentType() const
	{
		return EPICOSpatialECSComponentType::Unknown;
	}

	bool InitializeNativeComponent(const spatialpack::SceneEntity& InNativeEntity);

	bool HasNativeEntity() const;

	bool IsAddedAtRuntime() const;

	void MarkAddedAtRuntime();

#if PLATFORM_ANDROID
	const spatialpack::SceneEntity& GetCachedNativeEntity() const
	{
		return CachedNativeEntity;
	}
#endif

protected:
#if PLATFORM_ANDROID
	spatialpack::SceneEntity& GetCachedNativeEntity()
	{
		return CachedNativeEntity;
	}

	virtual void OnInitializeNativeComponent(spatialpack::SceneEntity& InNativeEntity) {}

	virtual void ApplyAllPropertiesToNative() {}

private:
	spatialpack::SceneEntity CachedNativeEntity;
	bool bHasCachedNativeEntity = false;
#endif

	UPROPERTY(Transient)
	bool bAddedAtRuntime = false;
};
