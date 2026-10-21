// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "PICOSpatialRuntimeSettings.h"
#include "Delegates/IDelegateInstance.h"
#include "PICOSpatialUtility.h"
#include "PICOSpatialContainer.generated.h"

// Forward declarations
class UBoxComponent;
class UArrowComponent;
class USceneComponent;
class FSceneEntityProxy;

UENUM(BlueprintType)
enum class EPICOSpatialWindowContainerCaptionBarStyle : uint8
{
	Default = 0,
	AutomaticHide = 1,
};

UENUM(BlueprintType)
enum class EPICOSpatialWindowContainerScaleType : uint8
{
	Automatic = 0,
	Dynamic = 1,
	Fixed = 2,
};

/**
 * PICO Spatial Container Actor
 * Manages spatial content within a defined volume and provides interaction with the PICO spatial system.
 * This actor serves as a boundary and management system for spatial content in PICO XR applications.
 */
UCLASS(ClassGroup = (PICOSpatial), BlueprintType, Blueprintable, Abstract)
class PICOSPATIAL_API APICOSpatialContainer : public ATriggerBox
{
	GENERATED_UCLASS_BODY()

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;
	
	/** Called when the actor is being destroyed */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/** Called every frame */
	virtual void Tick(float DeltaSeconds) override;

	/** Whether this is the default spatial container for the level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial", meta = (DisplayName = "Default Spatial Container", ToolTip = "Set to true if this is the default spatial container for the level"))
	bool bDefaultSpatialContainer = false;

	/** Type of spatial container when not using default settings */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial", meta = (DisplayName = "Spatial Container Type", EditCondition = "!bDefaultSpatialContainer", ToolTip = "Specifies the type of spatial container (Bounded or Unbounded)"))
	ESpatialContainerTypePICO SpatialContainerTypePICO = ESpatialContainerTypePICO::Bounded;

	/** Name of the spatial container */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO Spatial")
	FString SpatialContainerName;

	/** Style for unbounded spatial containers */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial", meta = (DisplayName = "Unbounded Style", EditCondition = "!bDefaultSpatialContainer && SpatialContainerTypePICO == ESpatialContainerTypePICO::Unbounded", ToolTip = "Defines the visual style for unbounded spatial containers"))
	EUnboundedStylePICO UnboundedStylePICO = EUnboundedStylePICO::AUTOMATIC;

	/** Size dimensions for bounded spatial containers (in centimeters) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial", meta = (DisplayName = "Container Size", EditCondition = "!bDefaultSpatialContainer && SpatialContainerTypePICO == ESpatialContainerTypePICO::Bounded", ToolTip = "Defines the size of the spatial container in centimeters"))
	FVector SpatialContainerSize = FVector(100.0f, 100.0f, 100.0f);
	
	/** Whether to automatically scale content to fit within the spatial container */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial", meta = (DisplayName = "Scale Content To Container", EditCondition = "!bDefaultSpatialContainer && SpatialContainerTypePICO == ESpatialContainerTypePICO::Bounded", ToolTip = "Automatically scales content to fit within the spatial container bounds"))
	bool bScaleContentToSpatialContainer = true;
	
	/** Whether to automatically open the spatial container when gameplay begins */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial", meta = (DisplayName = "Open At Begin Play", EditCondition = "!bDefaultSpatialContainer", ToolTip = "Automatically opens the spatial container at the start of gameplay"))
	bool bOpenSpatialContainerAtBegin = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial", meta = (DisplayName = "Caption Bar Style", EditCondition = "!bDefaultSpatialContainer && SpatialContainerTypePICO == ESpatialContainerTypePICO::Bounded"))
	EPICOSpatialWindowContainerCaptionBarStyle WindowContainerCaptionBarStyle = EPICOSpatialWindowContainerCaptionBarStyle::Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial", meta = (DisplayName = "Scale Type", EditCondition = "!bDefaultSpatialContainer && SpatialContainerTypePICO == ESpatialContainerTypePICO::Bounded"))
	EPICOSpatialWindowContainerScaleType WindowContainerScaleType = EPICOSpatialWindowContainerScaleType::Fixed;

	/** Scale factor for Image-Based Lighting (IBL) intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO Spatial|Lighting", meta = (DisplayName = "IBL Scale", ToolTip = "Controls the intensity of Image-Based Lighting within the spatial container", ClampMin = "0.0", ClampMax = "10.0"))
	float IBLScale = 0.24f;

	/**
	 * Whether this container automatically calls SetEnableWorldRendering when entering/leaving foreground.
	 * Disable this if you want to manage UE world rendering yourself, or if multiple containers
	 * conflict with each other's rendering state.
	 * Default: false (no automatic rendering control).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO Spatial", meta = (DisplayName = "Auto Manage World Rendering", ToolTip = "If enabled, this container will automatically call SetEnableWorldRendering when it enters or leaves the foreground. Disable if you want to control UE world rendering manually."))
	bool bAutoManageWorldRendering = false;

	/**
	 * How long (in seconds) the container may remain in the background before entity proxies are
	 * destroyed. While in background and before timeout, existing proxies are kept alive without
	 * running per-frame proxy updates. Set to 0 or a negative value to disable timeout destruction.
	 * Default: 30 seconds.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO Spatial", meta = (DisplayName = "Background Timeout Seconds", ToolTip = "Destroy entity proxies after the container has been in the background for this many seconds. Set to 0 or negative to disable timeout destruction.", ClampMin = "0.0"))
	float BackgroundAutoCloseSeconds = 30.0f;

	/** Whether to use the manually specified actor list instead of automatic detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO Spatial|Actor Filter", meta = (DisplayName = "Use Manual Actor List", ToolTip = "If enabled, only actors in the Manual Actor List will be processed. If disabled, all actors in the level will be automatically processed."))
	bool bUseManualActorList = false;

	/** List of actors to manually include in this spatial container. Only used if 'Use Manual Actor List' is enabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO Spatial|Actor Filter", meta = (DisplayName = "Manual Actor List", EditCondition = "bUseManualActorList", ToolTip = "Manually specify which actors should be processed by this spatial container. If empty and 'Use Manual Actor List' is enabled, no actors will be processed."))
	TArray<TObjectPtr<AActor>> ManualActorList;
	
	/**
	 * Opens the PICO spatial container for interaction.
	 * @return True if the container was successfully opened, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO Spatial", meta = (DisplayName = "Open Spatial Container"))
	bool OpenSpatialContainerPICO();

	/**
	 * Closes the PICO spatial container.
	 * @return True if the container was successfully closed, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO Spatial", meta = (DisplayName = "Close Spatial Container"))
	bool CloseSpatialContainerPICO();

	/**
	 * Pauses the PICO spatial container operations.
	 * @return True if the container was successfully paused, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO Spatial", meta = (DisplayName = "Pause Spatial Container"))
	bool PauseSpatialContainerPICO();

	/**
	 * Resumes the PICO spatial container operations after being paused.
	 * @return True if the container was successfully resumed, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO Spatial", meta = (DisplayName = "Resume Spatial Container"))
	bool ResumeSpatialContainerPICO();

	/**
	 * Sets the global Image-Based Lighting scale for the spatial container.
	 * @param Scale The scale factor for IBL intensity (typically 0.0 to 10.0)
	 * @return True if the IBL scale was successfully set, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|Lighting", meta = (DisplayName = "Set Global IBL Scale"))
	bool SetGlobalIBLScalePICO(float Scale = 1.0f);

	/**
	 * Adds an actor to the manual actor list.
	 * @param Actor The actor to add
	 * @return True if the actor was successfully added, false if it was already in the list or invalid
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|Actor Filter", meta = (DisplayName = "Add Actor To Manual List"))
	bool AddActorToManualList(AActor* Actor);

	/**
	 * Removes an actor from the manual actor list.
	 * @param Actor The actor to remove
	 * @return True if the actor was successfully removed, false if it wasn't in the list
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|Actor Filter", meta = (DisplayName = "Remove Actor From Manual List"))
	bool RemoveActorFromManualList(AActor* Actor);

	/**
	 * Clears all actors from the manual actor list.
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|Actor Filter", meta = (DisplayName = "Clear Manual Actor List"))
	void ClearManualActorList();

	/**
	 * Removes invalid (destroyed or pending kill) actors from the manual actor list.
	 * @return Number of invalid actors removed
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|Actor Filter", meta = (DisplayName = "Clean Invalid Actors From List"))
	int32 CleanInvalidActorsFromList();

	/** Whether the spatial container is currently in the process of opening */
	UPROPERTY(BlueprintReadOnly, Category = "PICO Spatial|Status", meta = (DisplayName = "Is Opening"))
	bool bIsSpatialContainerOpening = false;

	/** Whether the container has entered the foreground state */
	UPROPERTY(BlueprintReadOnly, Category = "PICO Spatial|Status", meta = (DisplayName = "Has Entered Foreground"))
	bool bHasEnterForeground = false;

	/** Whether the container currently has input focus */
	UPROPERTY(BlueprintReadOnly, Category = "PICO Spatial|Status", meta = (DisplayName = "Has Focus"))
	bool bHasFocus = false;

	/** Whether the container is currently visible to the user */
	UPROPERTY(BlueprintReadOnly, Category = "PICO Spatial|Status", meta = (DisplayName = "Is On Sight"))
	bool bIsOnSight = false;

	/** Unique identifier for this container in the PICO spatial system */
	uint64 ContainerID = static_cast<uint64>(-1);
	
	void* GameApplicationContext = nullptr;

	/** Pointer to the game activity instance (Android platform) */
	void* GameActivityInstance = nullptr;

	void* GameActivityClass = nullptr;
	
	/** Pointer to the PICO spatial activity instance (Android platform) */
	void* PICOSpatialActivityInstance = nullptr;
	
	/** Pointer to the PICO spatial activity class (Android platform) */
	void* PICOSpatialActivityClass = nullptr;
	
	/** Whether this container is currently bound to the spatial container system */
	bool bBindedToSpatialContainer = false;

	/** Accumulated time (seconds) the container has been continuously in the background. Reset on foreground. */
	float BackgroundElapsedSeconds = 0.0f;

	/** True when the container has been in the background longer than BackgroundAutoCloseSeconds.
	 *  Proxies are suppressed while this is set, allowing dead-proxy cleanup to destroy them.
	 *  Cleared automatically when the container returns to foreground. */
	bool bBackgroundTimeout = false;

	/**
	 * Callback when window container is created.
	 * @param Id The ID of the created window container
	 */
	void WindowContainerOnCreatedCallback(uint64 Id);
	
	/**
	 * Callback when window container is destroyed.
	 * @param Id The ID of the destroyed window container
	 */
	void WindowContainerOnDestroyedCallback(uint64 Id);
	
	/**
	 * Callback when window container enters foreground.
	 * @param Id The ID of the window container
	 */
	void WindowContainerOnForegroundCallback(uint64 Id);
	
	/**
	 * Callback when window container enters background.
	 * @param Id The ID of the window container
	 */
	void WindowContainerOnBackgroundCallback(uint64 Id);
	
	/**
	 * Callback when window container focus changes.
	 * @param Id The ID of the window container
	 * @param bInHasFocus Whether the container has focus
	 */
	void WindowContainerOnFocusCallback(uint64 Id, bool bInHasFocus);
	
	/**
	 * Callback when window container visibility changes.
	 * @param Id The ID of the window container
	 * @param bInIsOnSight Whether the container is visible
	 */
	void WindowContainerOnSightCallback(uint64 Id, bool bInIsOnSight);

protected:
	/**
	 * Callback when a managed actor in the manual list is being destroyed.
	 * Provides proactive cleanup by removing actors immediately when they're destroyed.
	 * @param Actor The actor that is being destroyed
	 */
	UFUNCTION()
	void OnManagedActorDestroyed(AActor* Actor);

public:
	/** Root 3D component for spatial positioning and orientation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PICO Spatial|Components", meta = (DisplayName = "Root 3D Component"))
	TObjectPtr<class UArrowComponent> Root3dComponent = nullptr;

	/** Socket component for directional light attachment */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PICO Spatial|Components", meta = (DisplayName = "Directional Light Socket"))
	TObjectPtr<class USceneComponent> DirectionalLightSocket = nullptr;

	/** Shared pointer to the root entity proxy for this spatial container */
	TSharedPtr<class FSceneEntityProxy> SpatialContainerRootEntityProxy = nullptr;
	
	/** Pointer to the native PICO spatial container (spatialpack::AppForm) */
	spatialpack::AppForm* SpatialContainer = nullptr;
	
	/** Counter for active spatial containers that require shared world-rendering management. */
	static int32 bNeedSharedRendering;

	/** Register an opened container by native AppForm ID for fast Android callback lookup. */
	static void RegisterOpenedContainer(uint64 Id, APICOSpatialContainer* Container);

	/** Remove an opened container registration. */
	static void UnregisterOpenedContainer(uint64 Id, APICOSpatialContainer* Container);

	/** Find an opened container by native AppForm ID without scanning the world. */
	static APICOSpatialContainer* FindOpenedContainer(uint64 Id);

private:
	/** Native AppForm ID -> container lookup used by Android activity callbacks. */
	static TMap<uint64, TWeakObjectPtr<APICOSpatialContainer>> OpenedContainersByID;

	/** Close the container and optionally destroy the native container object. */
	bool CloseSpatialContainerInternal(bool bDestroyNativeContainer);

	/** Reset background timeout state so reopen starts from a clean lifecycle. */
	void ResetBackgroundTimeoutState();

	/** Release Android global refs captured during open/bind. */
	void ReleaseAndroidGlobalRefs();

	/** Cached transform from the last tick for dirty checking */
	FTransform LastTranslation;
	
	/** Whether the container transform has changed and needs updating */
	uint8 bDirty : 1;
	
	/** Whether transform was dirty this frame (optimization flag) */
	uint8 bTransformDirtyThisFrame : 1;

	/** Cached box component reference to avoid repeated casts */
	UPROPERTY(Transient)
	TObjectPtr<UBoxComponent> CachedBoxComponent = nullptr;

	/** Delegate handle for window container created event */
	FDelegateHandle OnWindowContainerCreatedDelegateHandle;
	
	/** Delegate handle for window container destroyed event */
	FDelegateHandle OnWindowContainerDestroyedDelegateHandle;
	
	/** Delegate handle for window container foreground event */
	FDelegateHandle OnWindowContainerForegroundDelegateHandle;
	
	/** Delegate handle for window container background event */
	FDelegateHandle OnWindowContainerBackgroundDelegateHandle;
	
	/** Delegate handle for window container focus event */
	FDelegateHandle OnWindowContainerFocusDelegateHandle;
	
	/** Delegate handle for window container sight event */
	FDelegateHandle OnWindowContainerSightDelegateHandle;
};
