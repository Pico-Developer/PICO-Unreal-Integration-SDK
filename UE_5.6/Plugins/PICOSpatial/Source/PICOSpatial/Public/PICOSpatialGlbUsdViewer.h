// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PICOSpatialUtility.h"
#include "PICOSpatialGlbUsdViewer.generated.h"

/** 
 * Component for viewing 3D models within the PICO Spatial environment.
 * 
 * This component provides functionality to load and display 3D model files with integration
 * with the PICO Spatial Container system. It handles:
 * - Trying an explicit Android storage path or file URI first when provided at runtime
 * - Falling back to AssetsPICO/UserCustomAssets when the explicit path is invalid or missing
 * - Managing model visibility and integration with spatial containers
 * - Automatic cleanup and lifecycle management
 */
UCLASS(ClassGroup = (PICOSpatial), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class PICOSPATIAL_API UPICOSpatialGlbUsdViewer : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPICOSpatialGlbUsdViewer();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnVisibilityChanged() override;
	virtual void OnHiddenInGameChanged() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial", meta = (DisplayName = "Set Model Path"))
	void SetModelPath(const FString& InModelPath);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial", meta = (DisplayName = "Reload Model"))
	bool ReloadModel();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial", meta = (DisplayName = "Clear Model"))
	void ClearModel();

	UFUNCTION(BlueprintPure, Category = "PICO Spatial", meta = (DisplayName = "Get Model Path"))
	FString GetModelPath() const;

	/** Path to the 3D model file to be loaded and displayed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial", meta = (DisplayName = "Model Path", ToolTip = "Project fallback: choose a file under AssetsPICO/UserCustomAssets, or type a relative path/name there. Runtime API can also provide an Android storage absolute path or file URI. Load order is: explicit Android storage path first, then UserCustomAssets fallback."))
	FFilePath ModelPath;

	/** Reference to the PICO spatial container that will manage this model viewer */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial", meta = (DisplayName = "Spatial Container", ToolTip = "The spatial container responsible for managing this model"))
	TObjectPtr<class APICOSpatialContainer> Container;

private:
#if PLATFORM_ANDROID
	spatialpack::SceneEntity LoadedEntity;
#endif

	bool LoadModelEntity();
	void ReleaseLoadedEntity();
	void SyncLoadedEntityVisibility();
	void RefreshTickPolicy();
	bool bIsAttached = false;
};
