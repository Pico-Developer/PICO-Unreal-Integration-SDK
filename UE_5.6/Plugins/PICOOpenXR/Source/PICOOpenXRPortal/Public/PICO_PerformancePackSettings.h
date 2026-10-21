// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/RendererSettings.h"
#include "PICOOpenXRRuntimeSettings.h"
#include "PICO_PerformancePackSettings.generated.h"

UENUM()
enum class EPICOPerfPackRefreshRate : uint8
{
	Default UMETA(DisplayName = "Default"),
	Hz72 UMETA(DisplayName = "72Hz"),
	Hz90 UMETA(DisplayName = "90Hz"),
};

UENUM()
enum class EPICOPerfPackFoveationMode : uint8
{
	Off UMETA(DisplayName = "None"),
#ifdef PICO_CUSTOM_ENGINE
	EyeTracked UMETA(DisplayName = "Eyetracked Foveated Rendering"),
#endif
	Fixed UMETA(DisplayName = "Fixed Foveated Rendering"),
};

UENUM()
enum class EPICOPerfPackFoveationLevel : uint8
{
	None UMETA(DisplayName = "None"),
	Low UMETA(DisplayName = "Low"),
	Medium UMETA(DisplayName = "Medium"),
	High UMETA(DisplayName = "High"),
};

USTRUCT()
struct FPICOPerfPackDeviceSettings
{
	GENERATED_BODY()

	UPROPERTY(config, EditAnywhere, Category = "Device", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float ResolutionScale = 1.0f;

	UPROPERTY(config, EditAnywhere, Category = "Device")
	EPICOPerfPackRefreshRate RefreshRate = EPICOPerfPackRefreshRate::Default;

	UPROPERTY(config, EditAnywhere, Category = "Device")
	EPICOPerfPackFoveationMode FoveationMode = EPICOPerfPackFoveationMode::Off;

	UPROPERTY(config, EditAnywhere, Category = "Device")
	EPICOPerfPackFoveationLevel FoveationLevel = EPICOPerfPackFoveationLevel::Low;

	UPROPERTY(config, EditAnywhere, Category = "Features")
	bool bSuperResolution = true;

	UPROPERTY(config, EditAnywhere, Category = "Features")
	ESharpeningTypePICO SharpeningSetting = ESharpeningTypePICO::None;

	UPROPERTY(config, EditAnywhere, Category = "Features")
	ESharpeningEnhanceModePICO SharpeningEnhanceModeSetting = ESharpeningEnhanceModePICO::None;

	UPROPERTY(config, EditAnywhere, Category = "Features")
	bool bDynamicResolution = false;

	UPROPERTY(config, EditAnywhere, Category = "Features")
	EAdaptiveResolutionSettingPICO AdaptiveResolutionSetting = EAdaptiveResolutionSettingPICO::Balanced;

	UPROPERTY(config, EditAnywhere, Category = "Features")
	bool bTonemapSubpass = false;
};

UCLASS(config = Engine, defaultconfig)
class PICOOPENXRPORTAL_API UPICO_PerformancePackSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category = "BasicSettings", meta = (DisplayName = "Swan"))
	FPICOPerfPackDeviceSettings Swan;

	UPROPERTY(config, EditAnywhere, Category = "BasicSettings", meta = (DisplayName = "PICO 4 Series"))
	FPICOPerfPackDeviceSettings PICO4;

	UPROPERTY(config, EditAnywhere, Category = "BasicSettings", meta = (DisplayName = "Others"))
	FPICOPerfPackDeviceSettings Others;

	void ResetSwanToDefault();
	void ResetPICO4ToDefault();
	void ResetOthersToDefault();

	void ApplySwan();
	void ApplyPICO4();
	void ApplyOthers();

	bool IsBasicProfileEnabled(FName ProfilePropertyName) const;
	void SetBasicProfileEnabled(FName ProfilePropertyName, bool bEnabled);

	UPICO_PerformancePackSettings();

	UPROPERTY(config, EditAnywhere, Category = "AdvanceSettings", meta = (
		ConsoleVariable = "r.Mobile.ShadingPath", DisplayName = "Mobile Shading",
		ToolTip = "The shading path to use on mobile platforms. Changing this setting requires restarting the editor. Forward shading will force MSAA. Mobile HDR is required for Deferred Shading.",
		ConfigRestartRequired = true))
	TEnumAsByte<EMobileShadingPath::Type> MobileShadingPath = EMobileShadingPath::Forward;

	UPROPERTY(config, EditAnywhere, Category = "AdvanceSettings", meta = (
		ConsoleVariable = "r.Mobile.AntiAliasing", DisplayName = "Mobile Anti-Aliasing Method",
		ToolTip = "The mobile default anti-aliasing method."))
	TEnumAsByte<EMobileAntiAliasingMethod::Type> MobileAntiAliasing = EMobileAntiAliasingMethod::MSAA;

	UPROPERTY(config, EditAnywhere, Category = "AdvanceSettings", meta = (
		EditCondition = "MobileAntiAliasing == EMobileAntiAliasingMethod::MSAA",
		ConsoleVariable = "r.MSAACount", DisplayName = "MSAA Sample Count",
		ToolTip = "Default number of samples for MSAA."))
	TEnumAsByte<ECompositingSampleCount::Type> MSAASampleCount = ECompositingSampleCount::Four;

	UPROPERTY(config, EditAnywhere, Category = "AdvanceSettings", meta = (
		EditCondition = "MobileShadingPath == 0",
		ConsoleVariable = "r.MobileHDR", DisplayName = "Mobile HDR",
		ToolTip = "If true, mobile pipelines include a full post-processing pass with tonemapping. Disable this setting for a performance boost and to enable stereoscopic rendering optimizations. Changing this setting requires restarting the editor.",
		ConfigRestartRequired = true))
	uint32 bMobilePostProcessing : 1 = 0;

	UPROPERTY(config, EditAnywhere, Category = "AdvanceSettings", meta = (
		ConsoleVariable = "r.Mobile.EnableMovableSpotlightsShadow",
		DisplayName = "Support Movable SpotlightShadows",
		ToolTip = "Generate shaders for primitives to receive shadow from movable spotlights. Changing this setting requires restarting the editor.",
		ConfigRestartRequired = true))
	uint32 bMobileAllowMovableSpotlightShadows : 1 = 0;

	virtual void PostInitProperties() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	bool WriteDefaultDeviceProfilesIni() const;
	bool WriteDefaultDeviceProfile(const FString& ProfileName, const FPICOPerfPackDeviceSettings& Config) const;
	bool RemoveDefaultDeviceProfile(const FString& ProfileName) const;
	bool SyncDefaultDeviceProfilesFromToggles() const;
	bool WriteBaseDeviceProfilesMatchProfiles() const;

	UPROPERTY(config)
	bool bEnableSwanProfile = false;

	UPROPERTY(config)
	bool bEnablePICO4Profile = false;

	UPROPERTY(config)
	bool bEnableOthersProfile = false;

	bool bEnforcingAdvancedConstraints = false;
	bool bSyncingFromRendererSettings = false;

	void UpdateOnRenderingSettingsChanged(UObject* Obj, FPropertyChangedEvent& ChangeEvent);
};
