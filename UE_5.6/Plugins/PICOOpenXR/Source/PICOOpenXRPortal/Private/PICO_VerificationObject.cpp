// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_VerificationObject.h"
#include "Engine/RendererSettings.h"
#include "AndroidRuntimeSettings.h"
#include "PICO_PortalUtils.h"
#include "GameFramework/InputSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"

static void EnsureMacMobileMultiViewEnabledForPortalFix()
{
#if PLATFORM_MAC
	if (GConfig)
	{
		const FString MacEnginePath = FConfigCacheIni::NormalizeConfigIniPath(FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("Mac/MacEngine.ini")));
		GConfig->LoadFile(MacEnginePath);
		GConfig->SetBool(TEXT("/Script/Engine.RendererSettings"), TEXT("vr.MobileMultiView"), true, MacEnginePath);
		GConfig->Flush(false, MacEnginePath);
		GConfig->UnloadFile(MacEnginePath);
	}
#endif
}

UPICOVerificationObject::UPICOVerificationObject(): Level(EVerificationLevelPICO::Error), bNeedRestartEditor(false)
{
}

UPICOVerificationObject::UPICOVerificationObject(const FText& InDisplayName, const FText& InDescription,const EVerificationLevelPICO& InVerificationLevel,bool InNeedRestartEditor)
	: DisplayName(InDisplayName), Description(InDescription), Level(InVerificationLevel), bNeedRestartEditor(InNeedRestartEditor)
{
}

void UPICOVerificationObject::PerformVerificationFix()
{
}

bool UPICOVerificationObject::IsVerificationFixed()
{
	return false;
}

bool UPICOVerificationObject::RequiresEditorRestart()
{
	return bNeedRestartEditor;
}

void UPICOSetMSAAObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(URendererSettings,MobileAntiAliasing,EMobileAntiAliasingMethod::MSAA);
	PICOXR_UPDATE_SETTINGS(URendererSettings,MSAASampleCount,ECompositingSampleCount::Four);
}

bool UPICOSetMSAAObject::IsVerificationFixed()
{
	const URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	return Settings->MobileAntiAliasing == EMobileAntiAliasingMethod::MSAA
		&& Settings->MSAASampleCount == ECompositingSampleCount::Four;;
}

void UPICOEnableMultiviewObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(URendererSettings,bMobileMultiView,1);
	EnsureMacMobileMultiViewEnabledForPortalFix();
}

bool UPICOEnableMultiviewObject::IsVerificationFixed()
{
	return GetMutableDefault<URendererSettings>()->bMobileMultiView != 0;
}

void UPICOEnableMobileHDRObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(URendererSettings,bMobilePostProcessing,0);
}

bool UPICOEnableMobileHDRObject::IsVerificationFixed()
{
	const URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	return Settings->bMobilePostProcessing == 0||Settings->MobileShadingPath==1;
}

void UPICOEnableVulkanObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(UAndroidRuntimeSettings,bSupportsVulkan,true);
	PICOXR_UPDATE_SETTINGS(UAndroidRuntimeSettings,bBuildForES31,false);
}

bool UPICOEnableVulkanObject::IsVerificationFixed()
{
	const UAndroidRuntimeSettings* Settings = GetMutableDefault<UAndroidRuntimeSettings>();
	return Settings->bSupportsVulkan && !Settings->bBuildForES31;
}

void UPICOHalfPrecisionFloatObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(URendererSettings,MobileFloatPrecisionMode,EMobileFloatPrecisionMode::Half);
}

bool UPICOHalfPrecisionFloatObject::IsVerificationFixed()
{
	const URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	return Settings->MobileFloatPrecisionMode == EMobileFloatPrecisionMode::Half;
}

void UPICODisableAmbientOcclusionObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(URendererSettings,bMobileAmbientOcclusion,0);
}

bool UPICODisableAmbientOcclusionObject::IsVerificationFixed()
{
	const URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	return Settings->bMobileAmbientOcclusion == 0;
}

void UPICOEnableOcclusionCullingObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(URendererSettings,bOcclusionCulling,1);
}

bool UPICOEnableOcclusionCullingObject::IsVerificationFixed()
{
	const URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	return Settings->bOcclusionCulling;
}

void UPICODisableMovableSpotlightShadowsObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(URendererSettings,bMobileAllowMovableSpotlightShadows,0);
}

bool UPICODisableMovableSpotlightShadowsObject::IsVerificationFixed()
{
	const URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	return !Settings->bMobileAllowMovableSpotlightShadows||Settings->MobileShadingPath==1;
}

void UPICODisablePICOXRPluginObject::PerformVerificationFix()
{
	bApplied = FVerificationUtilsPICO::DisablePlugin(PluginName);
}

bool UPICODisablePICOXRPluginObject::IsVerificationFixed()
{
	return bApplied || !FVerificationUtilsPICO::IsPluginEnabled(PluginName);
}

void UPICOSetAndroidSDKMinimumObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(UAndroidRuntimeSettings,MinSDKVersion,MinAndroidAPILevel);
}

bool UPICOSetAndroidSDKMinimumObject::IsVerificationFixed()
{
	const UAndroidRuntimeSettings* Settings = GetMutableDefault<UAndroidRuntimeSettings>();
	return Settings->MinSDKVersion >= MinAndroidAPILevel;
}

void UPICOSetAndroidSDKTargetObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(UAndroidRuntimeSettings,TargetSDKVersion,TargetAndroidAPILevel);
}

bool UPICOSetAndroidSDKTargetObject::IsVerificationFixed()
{
	const UAndroidRuntimeSettings* Settings = GetMutableDefault<UAndroidRuntimeSettings>();
	return Settings->TargetSDKVersion >= TargetAndroidAPILevel;
}

bool UPICOSetArm64CPUObject::IsVerificationFixed()
{
	const UAndroidRuntimeSettings* Settings = GetMutableDefault<UAndroidRuntimeSettings>();
	return Settings->bBuildForArm64 && !Settings->bBuildForX8664;
}

void UPICOSetArm64CPUObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(UAndroidRuntimeSettings,bBuildForArm64,true);
	PICOXR_UPDATE_SETTINGS(UAndroidRuntimeSettings,bBuildForX8664,false);
}

void UPICODisableTouchInterfaceObject::PerformVerificationFix()
{
	PICOXR_UPDATE_SETTINGS(UInputSettings,DefaultTouchInterface,nullptr);
}

bool UPICODisableTouchInterfaceObject::IsVerificationFixed()
{
	const UInputSettings* Settings = GetDefault<UInputSettings>();
	return Settings->DefaultTouchInterface.IsNull();
}
