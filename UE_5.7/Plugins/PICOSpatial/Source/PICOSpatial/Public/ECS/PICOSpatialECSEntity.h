#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ECS/PICOSpatialECSComponentType.h"
#include "PICOSpatialUtility.h"
#include "PICOSpatialECSEntity.generated.h"

class UPICOSpatialECSComponent;
class UPICOSpatialECSTextComponent;
class UPICOSpatialECSTransformComponent;
class UPICOSpatialECSVideoPlayerComponent;
class UPICOSpatialECSModelComponent;
class UPICOSpatialECSGaussianSplattingComponent;
class APICOSpatialContainer;

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PICOSPATIAL_API UPICOSpatialECSEntity : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Entity")
	TArray<TObjectPtr<UPICOSpatialECSComponent>> Components;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Entity")
	UPICOSpatialECSComponent* GetComponentByType(EPICOSpatialECSComponentType Type) const;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Entity")
	bool HasComponent(EPICOSpatialECSComponentType Type) const;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Entity")
	UPICOSpatialECSComponent* AddComponent(TSubclassOf<UPICOSpatialECSComponent> ComponentClass);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Entity")
	bool RemoveComponentByType(EPICOSpatialECSComponentType Type);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Entity")
	bool RemoveComponent(UPICOSpatialECSComponent* Component);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	UPICOSpatialECSTextComponent* GetTextComponent() const;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	UPICOSpatialECSTextComponent* GetOrAddTextComponent();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Video Player Component")
	UPICOSpatialECSVideoPlayerComponent* GetVideoPlayerComponent() const;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Video Player Component")
	UPICOSpatialECSVideoPlayerComponent* GetOrAddVideoPlayerComponent();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Transform Component")
	UPICOSpatialECSTransformComponent* GetTransformComponent() const;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Transform Component")
	UPICOSpatialECSTransformComponent* GetOrAddTransformComponent();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Model Component")
	UPICOSpatialECSModelComponent* GetModelComponent() const;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Model Component")
	UPICOSpatialECSModelComponent* GetOrAddModelComponent();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Gaussian Splatting Component")
	UPICOSpatialECSGaussianSplattingComponent* GetGaussianSplattingComponent() const;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Gaussian Splatting Component")
	UPICOSpatialECSGaussianSplattingComponent* GetOrAddGaussianSplattingComponent();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Entity")
	bool InitializeNativeEntity(APICOSpatialContainer* Container, FString NativeName);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Entity")
	bool IsNativeEntityValid() const;

#if PLATFORM_ANDROID
	bool InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer, const spatialpack::SceneEntity& ParentEntity, FString NativeName);

	spatialpack::SceneEntity& GetNativeEntity()
	{
		return NativeEntity;
	}

	const spatialpack::SceneEntity& GetNativeEntity() const
	{
		return NativeEntity;
	}
#endif

	virtual void BeginDestroy() override;
	virtual void PostLoad() override;

#if PLATFORM_ANDROID
	void DestroyNativeEntity();
#endif

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditImport() override;

private:
	void ValidateComponents();
#endif

#if PLATFORM_ANDROID
private:
	spatialpack::SceneEntity NativeEntity;
	spatialpack::AppForm* LastSpatialContainer = nullptr;
	bool bNativeEntityAddedToContainer = false;
	bool bNativeComponentsInitialized = false;
#endif
};
