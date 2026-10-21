// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/WorldSettings.h"
#include "GameFramework/Actor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PICOSpatialFunctionLibrary.generated.h"

class UPrimitiveComponent;
class USceneComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogPICOSpatial, Log, All);

/** Enumeration for input action states used in PICO spatial interactions */
UENUM(BlueprintType)
enum class EInputActionPICO : uint8
{
	/** Input action has been triggered */
	Triggered,
	/** Input action has started */
	Started,
	/** Input action is currently ongoing */
	Ongoing,
	/** Input action has been canceled */
	Canceled,
	/** Input action has been completed */
	Completed
};

/** Enumeration for different asset types supported in PICO spatial */
UENUM(BlueprintType)
enum class EAssetTypePICO : uint8
{
	/** Unknown asset type */
	Unknown,
	/** Mesh asset type */
	Mesh,
	/** Material asset type */
	Material,
	/** 2D texture asset type */
	Texture2D,
	/** Niagara particle system asset type */
	Niagara,
};

/** Structure containing world dependency information for PICO spatial */
USTRUCT(BlueprintType)
struct PICOSPATIAL_API FWorldDependenciesPICO
{
	GENERATED_BODY();

	/** Dependency asset path -> asset type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO Spatial|Dependencies")
	TMap<FString, EAssetTypePICO> Dependencies;

	/** Default constructor */
	FWorldDependenciesPICO()
	{
		Dependencies.Reset();
	}
};

/** Delegate signature for component clicked events */
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams(FComponentOnClickedSignaturePICO, UEventManagerPICO, OnClicked, UPrimitiveComponent*, ClickedComponent, FVector, Location);

/** Delegate signature for component pressed events */
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams(FComponentOnPressedSignaturePICO, UEventManagerPICO, OnPressed, UPrimitiveComponent*, PressedComponent, FVector, Location);

/** Delegate signature for component released events */
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams(FComponentOnReleasedSignaturePICO, UEventManagerPICO, OnReleased, UPrimitiveComponent*, ReleasedComponent, FVector, Location);

/** Delegate signature for component hover events */
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_ThreeParams(FComponentOnHoverSignaturePICO, UEventManagerPICO, OnHover, UPrimitiveComponent*, HoverComponent, EInputActionPICO, InputAction, FVector, Location);

/** Delegate signature for component end hover events */
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FComponentOnEndHoverSignaturePICO, UEventManagerPICO, OnEndHover, UPrimitiveComponent*, EndHoverComponent);

/** Delegate signature for component drag events */
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_FourParams(FComponentOnDragSignaturePICO, UEventManagerPICO, OnDrag, UPrimitiveComponent*, DragComponent, EInputActionPICO, InputAction, FVector, StartLocation, FVector, Location);

/**
 * Fired when a USceneComponent has been registered as a PICO Spatial proxy entity and is now
 * rendered by the PICO Spatial system. Use this to suppress or adjust UE-side rendering.
 * @param Component  The component that was converted to a proxy.
 */
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FOnComponentProxyCreatedSignaturePICO, UEventManagerPICO, OnComponentProxyCreated, USceneComponent*, Component);

/**
 * Fired when a PICO Spatial proxy entity is destroyed and the component is no longer
 * rendered by the PICO Spatial system. Use this to restore UE-side rendering.
 * @param Component  The component whose proxy was destroyed. May be null if the component
 *                   was already GC'd before the proxy was cleaned up.
 */
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FOnComponentProxyDestroyedSignaturePICO, UEventManagerPICO, OnComponentProxyDestroyed, USceneComponent*, Component);

/**
 * Event manager class for handling PICO spatial input events.
 * This class manages delegates for various input events like clicks, hovers, and drags.
 */
UCLASS(BlueprintType)
class PICOSPATIAL_API UEventManagerPICO : public UObject
{
	GENERATED_BODY()

public:
	/** Delegate fired when a component is clicked in PICO spatial */
	UPROPERTY(BlueprintAssignable, Category = "PICO Spatial|Input Events", meta = (DisplayName = "On Component Clicked"))
	FComponentOnClickedSignaturePICO OnClicked;

	/** Delegate fired when a component is pressed in PICO spatial */
	UPROPERTY(BlueprintAssignable, Category = "PICO Spatial|Input Events", meta = (DisplayName = "On Component Pressed"))
	FComponentOnPressedSignaturePICO OnPressed;

	/** Delegate fired when a component is released in PICO spatial */
	UPROPERTY(BlueprintAssignable, Category = "PICO Spatial|Input Events", meta = (DisplayName = "On Component Released"))
	FComponentOnReleasedSignaturePICO OnReleased;

	/** Delegate fired when hovering over a component in PICO spatial */
	UPROPERTY(BlueprintAssignable, Category = "PICO Spatial|Input Events", meta = (DisplayName = "On Component Hover"))
	FComponentOnHoverSignaturePICO OnHover;

	/** Delegate fired when hovering ends over a component in PICO spatial */
	UPROPERTY(BlueprintAssignable, Category = "PICO Spatial|Input Events", meta = (DisplayName = "On Component End Hover"))
	FComponentOnEndHoverSignaturePICO OnEndHover;

	/** Delegate fired when dragging a component in PICO spatial */
	UPROPERTY(BlueprintAssignable, Category = "PICO Spatial|Input Events", meta = (DisplayName = "On Component Drag"))
	FComponentOnDragSignaturePICO OnDrag;

	/**
	 * Fired when a USceneComponent has been registered as a PICO Spatial proxy and is now
	 * rendered by the PICO Spatial system instead of the UE main pass.
	 * Bind this to suppress or replace UE-side rendering for the component.
	 */
	UPROPERTY(BlueprintAssignable, Category = "PICO Spatial|Proxy", meta = (DisplayName = "On Component Proxy Created"))
	FOnComponentProxyCreatedSignaturePICO OnComponentProxyCreated;

	/**
	 * Fired when a PICO Spatial proxy entity is destroyed and the component is no longer
	 * rendered by the PICO Spatial system. Bind this to restore UE-side rendering.
	 * Note: Component may be null if the component was already GC'd before proxy cleanup.
	 */
	UPROPERTY(BlueprintAssignable, Category = "PICO Spatial|Proxy", meta = (DisplayName = "On Component Proxy Destroyed"))
	FOnComponentProxyDestroyedSignaturePICO OnComponentProxyDestroyed;
};

/**
 * Blueprint function library for PICO Spatial functionality.
 * Provides static utility functions for managing PICO spatial features including:
 * - Event management
 * - Spatial system control
 * - Window management
 */
UCLASS()
class PICOSPATIAL_API UPICOSpatialFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Default constructor */
	UPICOSpatialFunctionLibrary();

	/** Static instance of the event manager for PICO spatial input events */
	static UEventManagerPICO* EventManagerPICO;
	
	/**
	 * Gets the singleton event manager instance for PICO spatial input events.
	 * @return Pointer to the event manager instance, creates one if it doesn't exist
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|Core", meta = (DisplayName = "Get Event Manager"))
	static UEventManagerPICO* GetEventManagerPICO();

	/**
	 * Enables or disables PICO spatial functionality for the current session.
	 * @param bEnable True to enable PICO spatial, false to disable
	 * @param bClearAllNativeCachedResources True to clear all cached native resources when disabling
	 * @return True if the operation succeeded, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|Core", meta = (DisplayName = "Enable PICO Spatial"))
	static bool EnablePICOSpatialPICO(bool bEnable, bool bClearAllNativeCachedResources = false);

	/**
	 * Gets whether PICO spatial is currently enabled.
	 * @return True if PICO spatial is enabled, false otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "PICO Spatial|Core", meta = (DisplayName = "Is PICO Spatial Enabled"))
	static bool IsPICOSpatialEnabledPICO();

	/**
	 * Enables or disables input for the default PICO spatial window.
	 * @param bEnable True to enable window input, false to disable
	 * @return True if the operation succeeded, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|Window", meta = (DisplayName = "Enable Default Window Input"))
	static bool EnableDefaultWindowInputPICO(bool bEnable);

	/**
	 * Modifies the properties of the default PICO spatial window.
	 * @param Alpha Window alpha percentage (0-100). Values below 100 enable transparent window mode.
	 * @param bCustomSize Whether to use custom size settings
	 * @param Width Custom width of the window in pixels
	 * @param Height Custom height of the window in pixels
	 * @param OffsetX Horizontal offset applied to the current window position
	 * @param OffsetY Vertical offset applied to the current window position
	 * @return True if modification succeeded, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|Window", meta = (DisplayName = "Modify Default Window"))
	static bool ModifyDefaultWindowPICO(int32 Alpha, bool bCustomSize, int32 Width, int32 Height, int32 OffsetX, int32 OffsetY);
};
