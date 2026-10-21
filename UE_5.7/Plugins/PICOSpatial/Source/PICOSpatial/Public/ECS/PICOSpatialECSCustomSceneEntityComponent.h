#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "PICOSpatialECSCustomSceneEntityComponent.generated.h"

class UPICOSpatialECSEntity;
class UArrowComponent;
class FCustomSceneEntityProxy;

UCLASS(ClassGroup = (PICOSpatial), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class PICOSPATIAL_API UCustomSceneEntityComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UCustomSceneEntityComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Entity")
	TObjectPtr<UPICOSpatialECSEntity> ECSEntity = nullptr;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Custom Scene Entity")
	bool HasECSEntity() const;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Custom Scene Entity")
	UPICOSpatialECSEntity* GetOrCreateECSEntity();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Custom Scene Entity")
	void ClearECSEntity();

#if PLATFORM_ANDROID
	void SetCustomSceneEntityProxy(FCustomSceneEntityProxy* InProxy);
#endif

private:
	UPROPERTY(Transient, DuplicateTransient, TextExportTransient)
	TObjectPtr<UArrowComponent> OrientationArrow = nullptr;

#if PLATFORM_ANDROID
	FCustomSceneEntityProxy* CustomSceneEntityProxy = nullptr;
#endif
};

