// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_VerificationObject.h"
#include "Engine/RendererSettings.h"
#include "AndroidRuntimeSettings.h"
#include "PXR_VerificationUtils.h"
#include "GameFramework/InputSettings.h"

UVerificationObject::UVerificationObject(): Level(EVerificationLevel::Error), bNeedRestartEditor(false)
{
}

UVerificationObject::UVerificationObject(const FText& InDisplayName, const FText& InDescription,const EVerificationLevel& InVerificationLevel,bool InNeedRestartEditor)
	: DisplayName(InDisplayName), Description(InDescription), Level(InVerificationLevel), bNeedRestartEditor(InNeedRestartEditor)
{
}

void UVerificationObject::PerformVerificationFix()
{
}

bool UVerificationObject::IsVerificationFixed()
{
	return false;
}

bool UVerificationObject::RequiresEditorRestart()
{
	return bNeedRestartEditor;
}

void USetMSAAObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(URendererSettings,MobileAntiAliasing,EMobileAntiAliasingMethod::MSAA);
	PICOXR_UPDATE_SETTINGS(URendererSettings,MSAASampleCount,ECompositingSampleCount::Four);
}

bool USetMSAAObject::IsVerificationFixed()
{
	const URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	return Settings->MobileAntiAliasing == EMobileAntiAliasingMethod::MSAA
		&& Settings->MSAASampleCount == ECompositingSampleCount::Four;;
}

void UEnableMultiviewObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(URendererSettings,bMobileMultiView,1);
}

bool UEnableMultiviewObject::IsVerificationFixed()
{
	return GetMutableDefault<URendererSettings>()->bMobileMultiView != 0;
}

void UEnableMobileHDRObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(URendererSettings,bMobilePostProcessing,0);
}

bool UEnableMobileHDRObject::IsVerificationFixed()
{
	const URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	return Settings->bMobilePostProcessing == 0||Settings->MobileShadingPath==1;
}

void UEnableVulkanObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(UAndroidRuntimeSettings,bSupportsVulkan,true);
	PICOXR_UPDATE_SETTINGS(UAndroidRuntimeSettings,bBuildForES31,false);
}

bool UEnableVulkanObject::IsVerificationFixed()
{
	const UAndroidRuntimeSettings* Settings = GetMutableDefault<UAndroidRuntimeSettings>();
	return Settings->bSupportsVulkan && !Settings->bBuildForES31;
}

void UHalfPrecisionFloatObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(URendererSettings,MobileFloatPrecisionMode,EMobileFloatPrecisionMode::Half);
}

bool UHalfPrecisionFloatObject::IsVerificationFixed()
{
	const URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	return Settings->MobileFloatPrecisionMode == EMobileFloatPrecisionMode::Half;
}

void UDisableAmbientOcclusionObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(URendererSettings,bMobileAmbientOcclusion,0);
}

bool UDisableAmbientOcclusionObject::IsVerificationFixed()
{
	const URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	return Settings->bMobileAmbientOcclusion == 0;
}

void UEnableOcclusionCullingObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(URendererSettings,bOcclusionCulling,1);
}

bool UEnableOcclusionCullingObject::IsVerificationFixed()
{
	const URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	return Settings->bOcclusionCulling;
}

void UDisableMovableSpotlightShadowsObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(URendererSettings,bMobileAllowMovableSpotlightShadows,0);
}

bool UDisableMovableSpotlightShadowsObject::IsVerificationFixed()
{
	const URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	return !Settings->bMobileAllowMovableSpotlightShadows||Settings->MobileShadingPath==1;
}

void UDisableOpenXRPluginObject::PerformVerificationFix()
{
	bApplied =FVerificationUtils::DisablePlugin(PluginName);
}

bool UDisableOpenXRPluginObject::IsVerificationFixed()
{
	return bApplied || !FVerificationUtils::IsPluginEnabled(PluginName);
}

void USetAndroidSDKMinimumObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(UAndroidRuntimeSettings,MinSDKVersion,MinAndroidAPILevel);
}

bool USetAndroidSDKMinimumObject::IsVerificationFixed()
{
	const UAndroidRuntimeSettings* Settings = GetMutableDefault<UAndroidRuntimeSettings>();
	return Settings->MinSDKVersion >= MinAndroidAPILevel;
}

void USetAndroidSDKTargetObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(UAndroidRuntimeSettings,TargetSDKVersion,TargetAndroidAPILevel);
}

bool USetAndroidSDKTargetObject::IsVerificationFixed()
{
	const UAndroidRuntimeSettings* Settings = GetMutableDefault<UAndroidRuntimeSettings>();
	return Settings->TargetSDKVersion >= TargetAndroidAPILevel;
}

bool USetArm64CPUObject::IsVerificationFixed()
{
	const UAndroidRuntimeSettings* Settings = GetMutableDefault<UAndroidRuntimeSettings>();
	return Settings->bBuildForArm64 && !Settings->bBuildForX8664;
}

void USetArm64CPUObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(UAndroidRuntimeSettings,bBuildForArm64,true);
	PICOXR_UPDATE_SETTINGS(UAndroidRuntimeSettings,bBuildForX8664,false);
}

void UDisableTouchInterfaceObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(UInputSettings,DefaultTouchInterface,nullptr);
}

bool UDisableTouchInterfaceObject::IsVerificationFixed()
{
	const UInputSettings* Settings = GetDefault<UInputSettings>();
	return Settings->DefaultTouchInterface.IsNull();
}
