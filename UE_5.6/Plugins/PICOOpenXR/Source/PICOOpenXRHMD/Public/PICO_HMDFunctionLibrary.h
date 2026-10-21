// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "GameFramework/WorldSettings.h"
#include "GameFrameWork/Actor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HeadMountedDisplayTypes.h"
#include "PICOOpenXRRuntimeSettings.h"
#include "PICO_HMDFunctionLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPICOOpenXRHMD, Log, All);

UENUM(BlueprintType)
enum class EPerfSettingsDomainPICO : uint8
{
	None = 0 UMETA(Hidden),
	CPU = 1,
	GPU = 2,
};

UENUM(BlueprintType)
enum class EPerfSettingsSubDomainPICO : uint8
{
	None = 0 UMETA(Hidden),
	Compositing = 1,
	Rendering = 2,
	Thermal = 3,
};

UENUM(BlueprintType)
enum class EPerfSettingsLevelPICO : uint8
{
	PowerSavings = 0,
	SustainedLow = 25,
	SustainedHigh = 50,
	Boost = 75,
};

UENUM(BlueprintType)
enum class EPerfSettingsNotificationLevelPICO : uint8
{
	Normal = 0,
	Warning = 25,
	Impaired = 75,
};

UENUM(BlueprintType)
enum class EBoundaryTypePICO : uint8
{
	OuterContour = 0,
	InscribedRectangle = 1,
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeviceDisplayRefreshRateChangedDelegatePICO, float, NewDisplayRefreshRate, float, OldDisplayRefreshRate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDevicePerformanceSettingsChangedDelegatePICO, EPerfSettingsDomainPICO, Domain, EPerfSettingsSubDomainPICO, SubDomain, EPerfSettingsNotificationLevelPICO, ToLevel, EPerfSettingsNotificationLevelPICO, FromLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMRCStatusChangedDelegatePICO, bool, Enable);

UCLASS(BlueprintType)
class PICOOPENXRHMD_API UDelegateManagerPICO : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
		FOnDeviceDisplayRefreshRateChangedDelegatePICO OnDeviceDisplayRefreshRateChanged;

	UPROPERTY(BlueprintAssignable)
		FOnDevicePerformanceSettingsChangedDelegatePICO OnDevicePerformanceSettingsChanged;

	UPROPERTY(BlueprintAssignable, Category = "PICO|HMD|MRC")
		FOnMRCStatusChangedDelegatePICO OnMRCStatusChanged;
};

UCLASS()
class PICOOPENXRHMD_API UHMDFunctionLibraryPICO : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
		
public:
	/**
	* Get predicted HMD or controller pose.
	*
	* @param UseDefaultTime	true if the default time should be used, false otherwise.
	* @param Timespan		when UseDefaultTime is false, this value will be used as the predicted display time.
	* 
	* @return				true if the function call was successful, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
	static bool GetDevicePoseForTimePICO(const EControllerHand Hand, bool UseDefaultTime, FTimespan Timespan, bool& OutTimeWasUsed, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityRadPerSec, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float InWorldToMetersScale);

	/**
	* Get the worn state of the HMD using XR_EXT_user_presence openxr extension.
	*
	* @param ResultValid	true if the function call was successful, false otherwise
	* @return				worn state of the HMD
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
	static EHMDWornState::Type GetHMDWornStatePICO(bool& ResultValid);

	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
	static bool GetFieldOfViewPICO(float& OutHFOVInDegrees, float& OutVFOVInDegrees);
	
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
	static bool GetInterpupillaryDistancePICO(float& IPD);

	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
		static bool GetSupportedDisplayRefreshRatesPICO(TArray<float>& DisplayRefreshRates);

	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
		static bool GetCurrentDisplayRefreshRatePICO(float& DisplayRefreshRate, bool DoubleCheck);

	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
		static bool SetDisplayRefreshRatePICO(float DisplayRefreshRate);

	static UDelegateManagerPICO* PICODelegateManager;
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
		static UDelegateManagerPICO* GetDelegateManagerPICO();

	/**
	* Currently, only support projection layer content protection.
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
		static void EnableContentProtectPICO(bool Enable);

	/**
	* Reference: https://registry.khronos.org/OpenXR/specs/1.1/html/xrspec.html#XR_EXT_performance_settings
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD", meta = (Domain = "CPU"))
		static bool SetPerformanceSettingsPICO(EPerfSettingsDomainPICO Domain, EPerfSettingsLevelPICO Level);

	/**
	* Sets 'base rotation' - the rotation that will be subtracted from
	* the actual HMD orientation.
	* Sets base position offset (in cm). The base position offset is the distance from the physical (0, 0, 0) position
	* to current HMD position (bringing the (0, 0, 0) point to the current HMD position)
	* Note, this vector is set by ResetPosition call; use this method with care.
	* The axis of the vector are the same as in Unreal: X - forward, Y - right, Z - up.
	*
	* @param Rotation			(in) Rotator object with base rotation
	* @param BaseOffset (in) the vector to be set as base offset, in cm.
	* @param Options			(in) specifies either position, orientation or both should be set.
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
	static void SetBaseRotationAndBaseOffsetPICO(FRotator Rotation, FVector BaseOffset, EOrientPositionSelector::Type Options);

	/**
	* Returns current base rotation and base offset.
	* The base offset is currently used base position offset, previously set by the
	* ResetPosition or SetBasePositionOffset calls. It represents a vector that translates the HMD's position
	* into (0,0,0) point, in cm.
	* The axis of the vector are the same as in Unreal: X - forward, Y - right, Z - up.
	*
	* @param OutRotation			(out) Rotator object with base rotation
	* @param OutBaseOffset	(out) base position offset, vector, in cm.
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
	static void GetBaseRotationAndBaseOffsetPICO(FRotator& OutRotation, FVector& OutBaseOffset);

	/**
	* Get the next display time. Used for GetDevicePoseForTimePICO API and etc.
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
	static FTimespan GetDisplayTimePICO();

	UFUNCTION(BlueprintCallable, Category = "PICO|HMD|Boundary")
	static bool IsStationaryBoundaryModePICO(bool& bIsStationary);

	UFUNCTION(BlueprintCallable, Category = "PICO|HMD|Boundary")
	static bool GetVirtualBoundaryStatusPICO(bool& bIsReady, bool& bIsEnable, bool& bIsVisible);

	UFUNCTION(BlueprintCallable, Category = "PICO|HMD|Boundary")
	static bool SetVirtualBoundaryEnablePICO(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "PICO|HMD|Boundary")
	static bool SetVirtualBoundaryVisiblePICO(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "PICO|HMD|Boundary")
	static bool SetVirtualBoundarySeeThroughVisiblePICO(bool bVisible);

	/**
	* Test the intersection between the point or node and the boundary.
	* 
	* @param bPoint			true if the point is used, false if the node is used.
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD|Boundary")
	static bool BoundaryIntersectPointOrNodePICO(bool bPoint, EControllerHand Node, FVector Point, EBoundaryTypePICO BoundaryType, bool& Valid, bool& IsTriggering, float& ClosestDistance, FVector& ClosestPoint, FVector& ClosestPointNormal, float InWorldToMetersScale = 100);

	UFUNCTION(BlueprintCallable, Category = "PICO|HMD|Boundary")
	static bool GetBoundaryGeometryPICO(EBoundaryTypePICO BoundaryType, bool& Valid, TArray<FVector>& Points, float InWorldToMetersScale = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "PICO|HMD|MRC", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void EnableMRCDebugModePICO(UObject* WorldContextObject, bool Enable, bool ViewInHMD, bool UseCustomTransform, const FTransform& Pose, bool UseCustomCameraInfo, int Width, int Height, float Fov);
	
	/**
	* Hidden foregroud MRC layer, you will can not see any virtual game object in front of reality scene.
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD|MRC", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void DisableMRCForegroundLayerPICO(UObject* WorldContextObject, bool Disable);

	/**
	* When MRC running, you can use this api to puase MRC without stop session.Useful when switching level to avoid some issue like crashing.
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD|MRC")
	static void PauseMRCPICO(bool Pause);

	/**
	* Get the current viewport size. When dynamic resolution is disabled, this is the same as GetCurrentRenderTargetSizePICO.
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
	static bool GetViewportSizePICO(FIntPoint& Size);

	/**
	* Get the default recommended render target size. For example, the recommended render target size for PICO 4U is 1920x1920.
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
	static FIntPoint GetDefaultRenderTargetSizePICO();

	/**
	* Get current render target size. This is the size of the render target used for rendering. Multiplied by the PixelDensity.
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
	static FIntPoint GetCurrentRenderTargetSizePICO();

	/**
	* Reference: https://docs.rainmeter.net/tips/colormatrix-guide/
	*/
	UFUNCTION(BlueprintPure, Category = "PICO|HMD")
	static bool SetProjectionLayerColorMatrix3x3fPICO(bool Enable, FVector3f ColumnA, FVector3f ColumnB, FVector3f ColumnC);

	/**
	* New EXT-based battery query API.
	* Compared with the legacy controller battery API, this interface supports querying
	* battery information for hands and HMD, and exposes more battery state details.
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|HMD")
	static bool GetBatteryStateDisplayPICO(EControllerHand Hand, bool& Valid, float& BatteryLevel, bool& Charging, bool& PluggedIn, bool& NoBattery);
};
