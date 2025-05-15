// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#include "PXR_HMDRuntimeSettings.h"
#if WITH_EDITOR
#include "Editor.h"
#endif
#include "Engine/RendererSettings.h"
#include "Interfaces/IPluginManager.h"

static TAutoConsoleVariable<int32> CVarWaitFrameAtGameFrameTail(
	TEXT("PICO.WaitFrameAtGameFrameTail"),
	0,
	TEXT("Whether to WaitFrame() at the end of the GameFrame!"),
	ECVF_Default);

UPICOXRSettings::UPICOXRSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	bEnablePSensor(false),
	bIsHMD3Dof(false),
	bEnableHomeKey(false),
	bIsController3Dof(false),
	HandTrackingSupport(EPICOXRHandTrackingSupport::ControllersOnly),
	bEnableAutoKeyMapping(true),
	bEnableLateLatching(false),
	bUseHWsRGBEncoding(true),
	bUseRecommendedMSAA(false),
	CoordinateType(EPICOXRCoordinateType::Local),
	bEnableEyeTracking(false),
	bEnableEyeTrackingCalibration(false),
	bEnableFaceTracking(false),
	bEnableBodyTracking(false),
	bUseAdvanceInterface(false),
	bUseContentProtect(false),
	bSplashScreenAutoShow(true),
	refreshRate(ERefreshRate::Default),
	FoveationRenderingMode(EFoveationRenderingMode::FixedFoveationRendering),
	FoveationLevel(EFoveationLevel::None),
	bEnableEyeTrackingFoveationRendering(false),
	bEnableSuperResolution(false),
	SharpeningSetting(EPICOXRSharpeningType::None),
	SharpeningEnhanceMode(EPICOXRSharpeningEnhanceModeType::None)
{
#if WITH_EDITOR
	ResetsRGBConfig();
#endif
	ApplyCVarSettingsFromIni(TEXT("/Script/PICOXRHMD.PICOXRSettings"), *GEngineIni, ECVF_SetByProjectSetting);

}

void UPICOXRSettings::PostInitProperties()
{
	Super::PostInitProperties();
	bUseHWsRGBEncoding = true;
}

#if WITH_EDITOR
bool UPICOXRSettings::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty->GetFName() == FName("bAdaptiveResolutionEnabled")
		|| InProperty->GetFName() == FName("AdaptiveResolutionPowerSetting"))
	{
		// Only enable AdaptiveRes when FFR is disabled
		if (FoveationLevel != EFoveationLevel::None)
		{
			return false;
		}

#ifndef PICO_CUSTOM_ENGINE
		// Only enable AdaptiveRes on PICO fork of Unreal Engine
		return false;
#endif // PICO_CUSTOM_ENGINE

		return true;
	}


	return true;
}

void UPICOXRSettings::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	bUsingOSSplash = FPaths::FileExists(OSSplashScreen.FilePath) ? true : false;

	Super::PostEditChangeProperty(PropertyChangedEvent);
	Super::UpdateDefaultConfigFile();
	HandlesRGBHWSupport();

    FString Dst = FPaths::ProjectDir() / TEXT("pico_splash.png");
	
	if (bUsingOSSplash)
	{
		IFileManager::Get().Copy(*Dst, *OSSplashScreen.FilePath, true);
	}
	else
	{
		IFileManager::Get().Delete(*Dst, true);
	}

	static const FName EnableAnchorName = GET_MEMBER_NAME_CHECKED(UPICOXRSettings, bEnableAnchor);
	if (PropertyChangedEvent.Property->GetFName() == EnableAnchorName)
	{
		if (bEnableAnchor)
		{
			bEnableVST = true;
			UpdateSinglePropertyInConfigFile(GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICOXRSettings, bEnableVST)), GetDefaultConfigFilename());
		}
	}

	static const FName SuperResolutionName = GET_MEMBER_NAME_CHECKED(UPICOXRSettings, bEnableSuperResolution);
	if (PropertyChangedEvent.Property->GetFName() == SuperResolutionName)
	{
		if (bEnableSuperResolution)
		{
			SharpeningSetting = EPICOXRSharpeningType::None;
			UpdateSinglePropertyInConfigFile(GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICOXRSettings, SharpeningSetting)), GetDefaultConfigFilename());
		}
	}

	static const FName SharpeningName = GET_MEMBER_NAME_CHECKED(UPICOXRSettings, SharpeningSetting);
	if (PropertyChangedEvent.Property->GetFName() == SharpeningName)
	{
		if (SharpeningSetting!=EPICOXRSharpeningType::None)
		{
			bEnableSuperResolution = false;
			UpdateSinglePropertyInConfigFile(GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICOXRSettings, bEnableSuperResolution)), GetDefaultConfigFilename());
		}
	}

	static const FName FoveationRenderingModeName = GET_MEMBER_NAME_CHECKED(UPICOXRSettings, FoveationRenderingMode);
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == FoveationRenderingModeName)
	{
		if (FoveationRenderingMode == EFoveationRenderingMode::EyeTrackingFoveationRendering)
		{
			bEnableEyeTrackingFoveationRendering = true;
			UpdateSinglePropertyInConfigFile(GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICOXRSettings, bEnableEyeTrackingFoveationRendering)), GetDefaultConfigFilename());
		}
	}

	static const FName EnableEyeTrackingFoveationName = GET_MEMBER_NAME_CHECKED(UPICOXRSettings, bEnableEyeTrackingFoveationRendering);
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == EnableEyeTrackingFoveationName)
	{
		if (bEnableEyeTrackingFoveationRendering == false)
		{
			FoveationRenderingMode = EFoveationRenderingMode::FixedFoveationRendering;
			UpdateSinglePropertyInConfigFile(GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICOXRSettings, FoveationRenderingMode)), GetDefaultConfigFilename());
		}
	}
}

void UPICOXRSettings::HandlesRGBHWSupport()
{
	URendererSettings* const Settings = GetMutableDefault<URendererSettings>();
	static auto* MobileUseHWsRGBEncodingCVAR = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Mobile.UseHWsRGBEncoding"));

	if (bUseHWsRGBEncoding != Settings->bMobileUseHWsRGBEncoding)
	{
		Settings->bMobileUseHWsRGBEncoding = bUseHWsRGBEncoding;
		Settings->UpdateSinglePropertyInConfigFile(Settings->GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(URendererSettings, bMobileUseHWsRGBEncoding)), GetDefaultConfigFilename());
	}

	if (MobileUseHWsRGBEncodingCVAR && MobileUseHWsRGBEncodingCVAR->GetInt() != (int)bUseHWsRGBEncoding)
	{
		MobileUseHWsRGBEncodingCVAR->Set((int)bUseHWsRGBEncoding);
	}
}

void UPICOXRSettings::ResetsRGBConfig()
{
	// To Prevent sRGB from being turned off by AndroidSetting
	FEditorDelegates::RefreshAllBrowsers.AddUObject(this,&UPICOXRSettings::HandlesRGBHWSupport);
}
#endif




