// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPICOOpenXRSettings, Log, All);

#include "PICOOpenXRRuntimeSettings.generated.h"

UENUM(BlueprintType)
enum class EDisplayRefreshRatePICO : uint8
{
	Default = 0,
	Rate72 = 1,
	Rate90 = 2,
	Rate120 = 3,
};

UENUM(BlueprintType, meta = (Categories = "PICO|MR"))
enum class ESpatialMeshLodPICO :uint8
{
	Low = 0,
	Medium = 1,
	High = 2
};

UENUM()
enum class ESharpeningTypePICO : uint8
{
	None UMETA(ToolTip = "Turn off Sharpening"),
	NormalSharpening UMETA(ToolTip = "Turn on NormalSharpening"),
	QualitySharpening UMETA(ToolTip = "Turn on QualitySharpening")
};

UENUM()
enum class ESharpeningEnhanceModePICO : uint8
{
	None UMETA(ToolTip = "Turn off all EnhanceMode"),
	FixedFoveated UMETA(ToolTip = "Turn on Fixed Foveated Mode"),
	Adaptive UMETA(ToolTip = "Turn on Adaptive"),
	Both UMETA(ToolTip = "Turn on Fixed Foveated and Adaptive Mode")
};

UENUM(BlueprintType)
enum class EAdaptiveResolutionSettingPICO : uint8
{
	HighQuality = 0,
	Balanced = 1,
	PowerSaving = 2,
};

UENUM(BlueprintType, meta = (Categories = "PICO|MR"))
enum class ESpatialLightEstimationResolutionPICO:uint8
{
	TextureResolution_8x8 = 0 UMETA(DisplayName = "8x8"),
	TextureResolution_16x16 = 1 UMETA(DisplayName = "16x16"),
	TextureResolution_32x32 = 2 UMETA(DisplayName = "32x32"),
	TextureResolution_64x64 = 3 UMETA(DisplayName = "64x64"),
	TextureResolution_128x128 = 4 UMETA(DisplayName = "128x128"),
};

UCLASS(config = Engine, defaultconfig)
class PICOOPENXRRUNTIMESETTINGS_API UPICOOpenXRRuntimeSettings : public UObject
{
public:
	GENERATED_UCLASS_BODY()

	virtual void PostInitProperties() override;

	static void ApplyCVarSettingsFromProjectSettings();

	static bool GetBoolConfigByKey(const FString& InKeyName);

	void ToggleOcclusionCulling();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif	  // WITH_EDITOR

	/** Whether eye tracking functionality can be used with the app */
	UPROPERTY(config, EditAnywhere, Category = Movement, meta = (DisplayName = "Enable Eye Tracking"))
	bool bEyeTrackingEnabled = false;

	UPROPERTY(config, EditAnywhere, Category = Movement, meta = (EditCondition = "bEyeTrackingEnabled", DisplayName = "Enable Eye Tracking Calibration"))
	bool bEnableEyeTrackingCalibration = false;

	/** Indicates whether FaceTracking is used in the current app */
	UPROPERTY(config, EditAnywhere, Category = Movement, meta = (DisplayName = "Enable Face Tracking"))
	bool bEnableFaceTracking = false;

	/** Indicates whether BodyTracking is used in the current app */
	UPROPERTY(config, EditAnywhere, Category = Movement, meta = (DisplayName = "Enable Body Tracking"))
	bool bEnableBodyTracking = false;

	/** Enable XR_PICO_expand_device PICO openxr extension */
	UPROPERTY(config, EditAnywhere, Category = Movement, meta = (DisplayName = "Enable Expand Device"))
	bool EnableExpandDevicesEXT = false;

	/** Enable XR_PICO_motion_tracking PICO openxr extension */
	UPROPERTY(config, EditAnywhere, Category = Movement, meta = (DisplayName = "Enable Motion Tracker"))
	bool EnableMotionTrackingEXT = false;

	/** Indicates whether HandTracking is used in the current app */
	UPROPERTY(config, EditAnywhere, Category = HandTracking, meta = (DisplayName = "Enable Hand Tracking"))
	bool bIsHandTrackingUsed = false;

	/** True if using XR_EXT_hand_interaction openxr extension. */
	UPROPERTY(config, EditAnywhere, Category = HandTracking, meta = (EditCondition = "bIsHandTrackingUsed", DisplayName = "Enable Hand Interaction"))
	bool bEnableHandInteractionEXT = true;

	/** True for supporting motion controller left and right motion source. */
	UPROPERTY(config, EditAnywhere, Category = HandTracking, meta = (EditCondition = "bIsHandTrackingUsed && bEnableHandInteractionEXT"))
	bool bSupportHandInteractionLeftRight = false;

	/** Enable some pico custom hand tracking blueprint api for more hand tracking data that built-in OpenXR Hand Tracking plugin does not support. */
	UPROPERTY(config, EditAnywhere, Category = HandTracking, meta = (EditCondition = "bIsHandTrackingUsed", DisplayName = "Use PICO Hand Tracking Module"))
	bool bIsPICOHandTrackingModuleUsed = false;

	/** If turned on, hand tracking will run at a higher tracking frequency, which will improve the smoothness of hand tracking, but the power consumption will increase. */
	UPROPERTY(config, EditAnywhere, Category = HandTracking, meta = (EditCondition = "bIsHandTrackingUsed", DisplayName = "Hand Tracking 60Hz"))
	bool bIsHandHighFrequencyTracking = false;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (DisplayName = "Enable OS Splash"))
	bool bUsingOSSplash;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (EditCondition = "bUsingOSSplash", DisplayName = "OS Splash Picture", FilePathFilter = "png"))
	FFilePath OSSplashScreen;

	/** RHI thread will not run on separated thread, combined with render thread. */
	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (DisplayName = "Disable Separate RHI Thread"))
	bool DisableRHIThread = true;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (DisplayName = "Setup Device Display Refresh Rate"))
	EDisplayRefreshRatePICO DisplayRefreshRate;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (DisplayName = "Set Tracking Origin To Local Floor Level"))
	bool bLocalFloorLevelEXT = false;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (DisplayName = "Enable Content Protect"))
	bool bContentProtectEXT = false;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (ConsoleVariable = "xr.OpenXRAllowDepthLayer", DisplayName = "Allow Depth Layer"))
	bool bAllowDepthLayer = false;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (ConsoleVariable = "xr.OpenXRFBFoveationDynamic", DisplayName = "Enable Dynamic FB Foveation"))
	bool bOpenXRFBFoveationDynamic = true;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (ConsoleVariable = "r.VRS.Enable", DisplayName = "Enable Variable Rate Shading"))
	bool bEnableVRS = true;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (ConsoleVariable = "xr.OpenXRInvertAlpha", DisplayName = "Invert OpenXR Alpha"))
	bool bOpenXRInvertAlpha = true;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (ConsoleVariable = "r.Vulkan.VRSFormat", ClampMin = "0", ClampMax = "5", UIMin = "0", UIMax = "5", DisplayName = "Vulkan VRS Format", ToolTip = "Maps to r.Vulkan.VRSFormat. 0-2 prefer/use/require Fragment Shading Rate, 3-5 prefer/use/require Fragment Density Map."))
	int32 VulkanVRSFormat = 3;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (ConsoleVariable = "r.Mobile.PICO.EnableSuperResolution", DisplayName = "Enable SuperResolution", ToolTip = "The two features, super resolution and sharpening, are mutually exclusive."))
	bool bEnableSuperResolution = false;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (ConsoleVariable = "r.Mobile.PICO.SharpeningSetting", DisplayName = "Sharpening Setting", ToolTip = "The two features, super resolution and sharpening, are mutually exclusive."))
	ESharpeningTypePICO SharpeningSetting = ESharpeningTypePICO::None;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (EditCondition = "SharpeningSetting!=ESharpeningTypePICO::None", ConsoleVariable = "r.Mobile.PICO.SharpeningEnhanceMode", DisplayName = "Sharpening EnhanceMode Setting", ToolTip = "FixedFoveated and Adaptive modes can be turned on in one or both modes."))
	ESharpeningEnhanceModePICO SharpeningEnhanceMode = ESharpeningEnhanceModePICO::None;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (ConsoleVariable = "xr.MobileLDRDynamicResolution", DisplayName = "Enable Dynamic Resolution"))
	bool bDynamicResolution = false;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (EditCondition = "bDynamicResolution", ConsoleVariable = "r.DynamicRes.OperationMode", ClampMin = "0", ClampMax = "2", UIMin = "0", UIMax = "2", DisplayName = "Dynamic Resolution Operation Mode"))
	int32 DynamicResolutionOperationMode = 2;

	UPROPERTY(config, EditAnywhere, Category = Feature)
	float MinimumDynamicResolutionScale = 0.6;

	UPROPERTY(config, EditAnywhere, Category = Feature)
	EAdaptiveResolutionSettingPICO AdaptiveResolutionSetting = EAdaptiveResolutionSettingPICO::Balanced;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (ToolTip = "See ProjectSettings -> Engine -> Rendering -> Culling -> Occlusion Culling"))
	bool bDisableOcclusionCulling = true;

	UPROPERTY(config, EditAnywhere, Category = Feature, meta = (DisplayName = "Disable OpenXR On Android Without Oculus", ToolTip = "Used by PICOOpenXRHMD module startup to set xr.DisableOpenXROnAndroidWithoutOculus. Keep disabled by default for PICO."))
	bool bDisableOpenXROnAndroidWithoutOculus = false;

	UPROPERTY(config, EditAnywhere, Category = MixedReality, meta = (DisplayName = "Enable MR Safeguard", ToolTip = "  MR safety, if you choose this option, your application will adopt MR safety policies during runtime. If not selected, it will continue to use VR safety policies by default."))
	bool bEnableMRSafeguard;

	UPROPERTY(config, EditAnywhere, Category = MixedReality, meta = (DisplayName = "Enable Video Seethrough"))
	bool bEnableVST;

	UPROPERTY(config, EditAnywhere, Category = MixedReality, meta = (DisplayName = "Enable Spatial Anchor"))
	bool bEnableAnchor;

	UPROPERTY(config, EditAnywhere, Category = MixedReality, meta = (EditCondition = "bEnableAnchor", DisplayName = "Enable Spatial Shared Anchor"))
	bool bEnableCloudAnchor;

	UPROPERTY(config, EditAnywhere, Category = MixedReality, meta = (DisplayName = "Enable Scene Capture"))
	bool bEnableSceneCapture;

	UPROPERTY(config, EditAnywhere, Category = MixedReality, meta = (DisplayName = "Enable Spatial Plane"))
	bool bEnablePlane;
	
	UPROPERTY(config, EditAnywhere, Category = MixedReality, meta = (DisplayName = "Enable Spatial Mesh"))
	bool bEnableMesh;

	UPROPERTY(config, EditAnywhere, Category = MixedReality, meta = (EditCondition = "bEnableMesh", DisplayName = "Enable Semantics Align With Triangle"))
	bool bSemanticsAlignWithTriangle;

	UPROPERTY(config, EditAnywhere, Category = MixedReality, meta = (EditCondition = "bEnableMesh", DisplayName = "Enable Semantics Align With Vertex"))
	bool bSemanticsAlignWithVertex;

	UPROPERTY(config, EditAnywhere, Category = MixedReality, meta = (EditCondition = "bEnableMesh", DisplayName = "Spatial Mesh Lod"))
	ESpatialMeshLodPICO MeshLod;

	UPROPERTY(Config, EditAnywhere, Category = MixedReality, Meta = (DisplayName = "Enable Light Estimation"))
		bool bEnableLightEstimation;

	UPROPERTY(Config, EditAnywhere, Category = MixedReality, Meta = (EditCondition = "bEnableLightEstimation", DisplayName = "Light Estimation Resolution"))
		ESpatialLightEstimationResolutionPICO LightEstimationResolution;
};
