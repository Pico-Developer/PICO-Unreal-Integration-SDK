// Copyright® PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/RendererSettings.h"
#include "PXR_VerificationSettings.generated.h"

/**
 * 
 */
UCLASS(config = EditorPerProjectUserSettings)
class PICOXRPROJECTVERIFICATION_API UPXR_VerificationSettings : public UObject
{
	GENERATED_BODY()
	/**
	 * @brief If Portal Auto showed
	 */
public:
	UPROPERTY(config, EditAnywhere, Category = PICOXR)
	bool bShowPortalOnEditorStart = true;
	
};

UENUM()
namespace EUnrealEngineType
{
	enum Type : int
	{
		Official_Engine = 0 UMETA(DisplayName = "Official Engine"),
		PICO_Custom_Engine = 1 UMETA(DisplayName = "PICO Custom Engine"),
	};
}

UCLASS(config = Engine, defaultconfig)
class PICOXRPROJECTVERIFICATION_API UPXR_RenderSettings : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(config, EditAnywhere,Category = PICO_Common, meta = (EditCondition = "false"))
	TEnumAsByte<EUnrealEngineType::Type> TypeEngine=EUnrealEngineType::Type::Official_Engine;
	
	UPROPERTY(config, EditAnywhere, Category = PICO_Common, meta = (
		ConsoleVariable = "r.Mobile.AntiAliasing", DisplayName = "Mobile Anti-Aliasing Method",
		ToolTip = "The mobile default anti-aliasing method."))
	TEnumAsByte<EMobileAntiAliasingMethod::Type> MobileAntiAliasing;

	UPROPERTY(config, EditAnywhere, Category = PICO_Common, meta = (
		ConsoleVariable = "r.MSAACount", DisplayName = "MSAA Sample Count",
		ToolTip = "Default number of samples for MSAA."))
	TEnumAsByte<ECompositingSampleCount::Type> MSAASampleCount;
	
	UPROPERTY(config, EditAnywhere, Category = PICO_CustomEngine, meta = (
		ToolTip = "The shading path to use on mobile platforms. Changing this setting requires restarting the editor. Mobile HDR is required for Deferred Shading.",
		EditCondition = "TypeEngine == 1",
		ConfigRestartRequired = true))
	TEnumAsByte<EMobileShadingPath::Type> MobileShadingPath;

	UPROPERTY(config, EditAnywhere, Category = PICO_Common, meta = (
		ConsoleVariable = "vr.MobileMultiView", DisplayName = "Mobile Multi-View",
		ToolTip = "Enable single-pass stereoscopic rendering on mobile platforms.",
		ConfigRestartRequired = true))
	uint32 bMobileMultiView : 1;
	
	UPROPERTY(config, EditAnywhere, Category = PICO_CustomEngine, meta=(
		EditCondition = "TypeEngine == 1",
		ConsoleVariable="r.MobileHDR", DisplayName="Mobile HDR",
		ToolTip="If true, mobile pipelines include a full post-processing pass with tonemapping. Disable this setting for a performance boost and to enable stereoscopic rendering optimizations. Changing this setting requires restarting the editor.",
		ConfigRestartRequired = true))
	uint32 bMobilePostProcessing:1;

	UPROPERTY(config, EditAnywhere, Category = PICO_CustomEngine, meta = (
		ConsoleVariable = "r.Mobile.EnableMovableSpotlightsShadow",
		DisplayName = "Support Movable SpotlightShadows",
		ToolTip = "Generate shaders for primitives to receive shadow from movable spotlights. Changing this setting requires restarting the editor.",
		EditCondition = "TypeEngine == 1",
		ConfigRestartRequired = true))
	uint32 bMobileAllowMovableSpotlightShadows : 1;
	
	UPROPERTY(config, EditAnywhere, Category = PICO_CustomEngine, meta = (
		ConsoleVariable = "vr.SupportMobileSpaceWarp", DisplayName = "Support Mobile Application Space Warp",
		ToolTip = "Allow to generate motion vector and depth buffer correctly for mobile application space warp).",
		EditCondition = "TypeEngine == 1",
		ConfigRestartRequired = true))
	uint32 bMobileSupportSpaceWarp : 1;

	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void UpdateOnRenderingSettingsChanged(UObject* Obj, FPropertyChangedEvent& ChangeEvent);
};