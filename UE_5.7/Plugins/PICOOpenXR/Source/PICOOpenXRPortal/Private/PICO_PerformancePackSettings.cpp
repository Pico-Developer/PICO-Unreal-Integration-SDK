// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_PerformancePackSettings.h"

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "PICO_PortalUtils.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UPICO_PerformancePackSettings::UPICO_PerformancePackSettings()
{
	Swan.ResolutionScale = 1.0f;
	Swan.RefreshRate = EPICOPerfPackRefreshRate::Hz90;
#ifdef PICO_CUSTOM_ENGINE
	Swan.FoveationMode = EPICOPerfPackFoveationMode::EyeTracked;
#else
	Swan.FoveationMode = EPICOPerfPackFoveationMode::Fixed;
#endif
	Swan.FoveationLevel = EPICOPerfPackFoveationLevel::Low;
	Swan.bSuperResolution = false;
	Swan.SharpeningSetting = ESharpeningTypePICO::None;
	Swan.SharpeningEnhanceModeSetting = ESharpeningEnhanceModePICO::None;
	Swan.bDynamicResolution = false;
	Swan.AdaptiveResolutionSetting = EAdaptiveResolutionSettingPICO::Balanced;
	Swan.bTonemapSubpass = false;

	PICO4.ResolutionScale = 1.0f;
	PICO4.RefreshRate = EPICOPerfPackRefreshRate::Hz72;
	PICO4.FoveationMode = EPICOPerfPackFoveationMode::Fixed;
	PICO4.FoveationLevel = EPICOPerfPackFoveationLevel::High;
	PICO4.bSuperResolution = false;
	PICO4.SharpeningSetting = ESharpeningTypePICO::None;
	PICO4.SharpeningEnhanceModeSetting = ESharpeningEnhanceModePICO::None;
	PICO4.bDynamicResolution = false;
	PICO4.AdaptiveResolutionSetting = EAdaptiveResolutionSettingPICO::Balanced;
	PICO4.bTonemapSubpass = false;

	Others.ResolutionScale = 1.0f;
	Others.RefreshRate = EPICOPerfPackRefreshRate::Default;
	Others.FoveationMode = EPICOPerfPackFoveationMode::Off;
	Others.FoveationLevel = EPICOPerfPackFoveationLevel::Low;
	Others.bSuperResolution = false;
	Others.SharpeningSetting = ESharpeningTypePICO::None;
	Others.SharpeningEnhanceModeSetting = ESharpeningEnhanceModePICO::None;
	Others.bDynamicResolution = false;
	Others.AdaptiveResolutionSetting = EAdaptiveResolutionSettingPICO::Balanced;
	Others.bTonemapSubpass = false;
}

static FString PICOPerfPackToString(EPICOPerfPackRefreshRate Value)
{
	switch (Value)
	{
	case EPICOPerfPackRefreshRate::Default: return TEXT("Default");
	case EPICOPerfPackRefreshRate::Hz72: return TEXT("72Hz");
	case EPICOPerfPackRefreshRate::Hz90: return TEXT("90Hz");
	default: return TEXT("90Hz");
	}
}

static FString PICOPerfPackToString(EPICOPerfPackFoveationMode Value)
{
	switch (Value)
	{
	case EPICOPerfPackFoveationMode::Off: return TEXT("None");
#ifdef PICO_CUSTOM_ENGINE
	case EPICOPerfPackFoveationMode::EyeTracked: return TEXT("Eyetracked Foveated Rendering");
#endif
	case EPICOPerfPackFoveationMode::Fixed: return TEXT("Fixed Foveated Rendering");
	default: return TEXT("None");
	}
}

static FString PICOPerfPackToString(EPICOPerfPackFoveationLevel Value)
{
	switch (Value)
	{
	case EPICOPerfPackFoveationLevel::None: return TEXT("None");
	case EPICOPerfPackFoveationLevel::Low: return TEXT("Low");
	case EPICOPerfPackFoveationLevel::Medium: return TEXT("Medium");
	case EPICOPerfPackFoveationLevel::High: return TEXT("High");
	default: return TEXT("None");
	}
}

static FString PICOPerfPackBool01(bool bValue)
{
	return bValue ? TEXT("1") : TEXT("0");
}

void UPICO_PerformancePackSettings::PostInitProperties()
{
	UObject::PostInitProperties();

	URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	if (!Settings)
	{
		return;
	}

	Settings->OnSettingChanged().AddUObject(this, &UPICO_PerformancePackSettings::UpdateOnRenderingSettingsChanged);

	MobileShadingPath = Settings->MobileShadingPath;
	MobileAntiAliasing = Settings->MobileAntiAliasing;
	MSAASampleCount = Settings->MSAASampleCount;
	bMobilePostProcessing = Settings->bMobilePostProcessing;
	bMobileAllowMovableSpotlightShadows = Settings->bMobileAllowMovableSpotlightShadows;

	const FProperty* MobileShadingPathProperty = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, MobileShadingPath));
	const FProperty* MobileAntiAliasingProperty = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, MobileAntiAliasing));
	const FProperty* MSAASampleCountProperty = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, MSAASampleCount));
	const FProperty* MobileHDRProperty = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, bMobilePostProcessing));
	const FProperty* MovableSpotlightShadowProperty = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, bMobileAllowMovableSpotlightShadows));
	if (!MobileShadingPathProperty || !MobileAntiAliasingProperty || !MSAASampleCountProperty || !MobileHDRProperty || !MovableSpotlightShadowProperty)
	{
		return;
	}
	UpdateSinglePropertyInConfigFile(MobileShadingPathProperty, GetDefaultConfigFilename());
	UpdateSinglePropertyInConfigFile(MobileAntiAliasingProperty, GetDefaultConfigFilename());
	UpdateSinglePropertyInConfigFile(MSAASampleCountProperty, GetDefaultConfigFilename());
	UpdateSinglePropertyInConfigFile(MobileHDRProperty, GetDefaultConfigFilename());
	UpdateSinglePropertyInConfigFile(MovableSpotlightShadowProperty, GetDefaultConfigFilename());

#if WITH_EDITOR
	if (!IsRunningCommandlet())
	{
		WriteBaseDeviceProfilesMatchProfiles();
		SyncDefaultDeviceProfilesFromToggles();
	}
#endif
}

void UPICO_PerformancePackSettings::UpdateOnRenderingSettingsChanged(UObject* Obj, FPropertyChangedEvent& ChangeEvent)
{
	if (bSyncingFromRendererSettings || bEnforcingAdvancedConstraints)
	{
		return;
	}

	const URendererSettings* RendererSettings = GetDefault<URendererSettings>();
	if (!RendererSettings || !ChangeEvent.Property)
	{
		return;
	}

	const FName ChangedName = ChangeEvent.Property->GetFName();
	const bool bAAChanged = ChangedName == GET_MEMBER_NAME_CHECKED(URendererSettings, MobileAntiAliasing);
	const bool bMSAAChanged = ChangedName == GET_MEMBER_NAME_CHECKED(URendererSettings, MSAASampleCount);
	const bool bHDRChanged = ChangedName == GET_MEMBER_NAME_CHECKED(URendererSettings, bMobilePostProcessing);
	const bool bShadingChanged = ChangedName == GET_MEMBER_NAME_CHECKED(URendererSettings, MobileShadingPath);
	const bool bSpotlightChanged = ChangedName == GET_MEMBER_NAME_CHECKED(URendererSettings, bMobileAllowMovableSpotlightShadows);

	if (!bAAChanged && !bMSAAChanged && !bHDRChanged && !bShadingChanged && !bSpotlightChanged)
	{
		return;
	}

	TGuardValue<bool> SyncGuard(bSyncingFromRendererSettings, true);
	TGuardValue<bool> ConstraintGuard(bEnforcingAdvancedConstraints, true);

	if (bShadingChanged)
	{
		MobileShadingPath = RendererSettings->MobileShadingPath;
		const FProperty* Property = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, MobileShadingPath));
		if (Property)
		{
			UpdateSinglePropertyInConfigFile(Property, GetDefaultConfigFilename());
			FPropertyChangedEvent LocalEvent(const_cast<FProperty*>(Property), EPropertyChangeType::ValueSet);
			FCoreUObjectDelegates::OnObjectPropertyChanged.Broadcast(this, LocalEvent);
		}
	}

	if (bAAChanged)
	{
		MobileAntiAliasing = RendererSettings->MobileAntiAliasing;
		if (MobileAntiAliasing == EMobileAntiAliasingMethod::TemporalAA && !RendererSettings->bMobilePostProcessing)
		{
			PICOXR_UPDATE_SETTINGS(URendererSettings, bMobilePostProcessing, true);
			bMobilePostProcessing = true;
			const FProperty* MobileHDRProperty = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, bMobilePostProcessing));
			if (MobileHDRProperty)
			{
				UpdateSinglePropertyInConfigFile(MobileHDRProperty, GetDefaultConfigFilename());
				FPropertyChangedEvent HDRChangedEvent(const_cast<FProperty*>(MobileHDRProperty), EPropertyChangeType::ValueSet);
				FCoreUObjectDelegates::OnObjectPropertyChanged.Broadcast(this, HDRChangedEvent);
			}
		}

		const FProperty* MobileAAProperty = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, MobileAntiAliasing));
		if (MobileAAProperty)
		{
			UpdateSinglePropertyInConfigFile(MobileAAProperty, GetDefaultConfigFilename());
			FPropertyChangedEvent LocalEvent(const_cast<FProperty*>(MobileAAProperty), EPropertyChangeType::ValueSet);
			FCoreUObjectDelegates::OnObjectPropertyChanged.Broadcast(this, LocalEvent);
		}
	}

	if (bHDRChanged)
	{
		bMobilePostProcessing = RendererSettings->bMobilePostProcessing;
		if (!bMobilePostProcessing && RendererSettings->MobileAntiAliasing == EMobileAntiAliasingMethod::TemporalAA)
		{
			PICOXR_UPDATE_SETTINGS(URendererSettings, MobileAntiAliasing, EMobileAntiAliasingMethod::MSAA);
			MobileAntiAliasing = EMobileAntiAliasingMethod::MSAA;
			const FProperty* MobileAAProperty = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, MobileAntiAliasing));
			if (MobileAAProperty)
			{
				UpdateSinglePropertyInConfigFile(MobileAAProperty, GetDefaultConfigFilename());
				FPropertyChangedEvent AAChangedEvent(const_cast<FProperty*>(MobileAAProperty), EPropertyChangeType::ValueSet);
				FCoreUObjectDelegates::OnObjectPropertyChanged.Broadcast(this, AAChangedEvent);
			}
		}

		const FProperty* MobileHDRProperty = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, bMobilePostProcessing));
		if (MobileHDRProperty)
		{
			UpdateSinglePropertyInConfigFile(MobileHDRProperty, GetDefaultConfigFilename());
			FPropertyChangedEvent LocalEvent(const_cast<FProperty*>(MobileHDRProperty), EPropertyChangeType::ValueSet);
			FCoreUObjectDelegates::OnObjectPropertyChanged.Broadcast(this, LocalEvent);
		}
	}

	if (bMSAAChanged)
	{
		MSAASampleCount = RendererSettings->MSAASampleCount;
		const FProperty* Property = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, MSAASampleCount));
		if (Property)
		{
			UpdateSinglePropertyInConfigFile(Property, GetDefaultConfigFilename());
			FPropertyChangedEvent LocalEvent(const_cast<FProperty*>(Property), EPropertyChangeType::ValueSet);
			FCoreUObjectDelegates::OnObjectPropertyChanged.Broadcast(this, LocalEvent);
		}
	}

	if (bSpotlightChanged)
	{
		bMobileAllowMovableSpotlightShadows = RendererSettings->bMobileAllowMovableSpotlightShadows;
		const FProperty* Property = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, bMobileAllowMovableSpotlightShadows));
		if (Property)
		{
			UpdateSinglePropertyInConfigFile(Property, GetDefaultConfigFilename());
			FPropertyChangedEvent LocalEvent(const_cast<FProperty*>(Property), EPropertyChangeType::ValueSet);
			FCoreUObjectDelegates::OnObjectPropertyChanged.Broadcast(this, LocalEvent);
		}
	}
}

static FPICOPerfPackDeviceSettings PICOPerfPackMakeSwanPreset()
{
	FPICOPerfPackDeviceSettings Settings;
	Settings.ResolutionScale = 1.0f;
	Settings.RefreshRate = EPICOPerfPackRefreshRate::Hz90;
#ifdef PICO_CUSTOM_ENGINE
	Settings.FoveationMode = EPICOPerfPackFoveationMode::EyeTracked;
#else
	Settings.FoveationMode = EPICOPerfPackFoveationMode::Fixed;
#endif
	Settings.FoveationLevel = EPICOPerfPackFoveationLevel::Low;
	Settings.bSuperResolution = false;
	Settings.SharpeningSetting = ESharpeningTypePICO::None;
	Settings.SharpeningEnhanceModeSetting = ESharpeningEnhanceModePICO::None;
	Settings.bDynamicResolution = false;
	Settings.AdaptiveResolutionSetting = EAdaptiveResolutionSettingPICO::Balanced;
	Settings.bTonemapSubpass = false;
	return Settings;
}

static FPICOPerfPackDeviceSettings PICOPerfPackMakePICO4Preset()
{
	FPICOPerfPackDeviceSettings Settings;
	Settings.ResolutionScale = 1.0f;
	Settings.RefreshRate = EPICOPerfPackRefreshRate::Hz72;
	Settings.FoveationMode = EPICOPerfPackFoveationMode::Fixed;
	Settings.FoveationLevel = EPICOPerfPackFoveationLevel::High;
	Settings.bSuperResolution = false;
	Settings.SharpeningSetting = ESharpeningTypePICO::None;
	Settings.SharpeningEnhanceModeSetting = ESharpeningEnhanceModePICO::None;
	Settings.bDynamicResolution = false;
	Settings.AdaptiveResolutionSetting = EAdaptiveResolutionSettingPICO::Balanced;
	Settings.bTonemapSubpass = false;
	return Settings;
}

static FPICOPerfPackDeviceSettings PICOPerfPackMakeOthersPreset()
{
	FPICOPerfPackDeviceSettings Settings;
	Settings.ResolutionScale = 1.0f;
	Settings.RefreshRate = EPICOPerfPackRefreshRate::Default;
	Settings.FoveationMode = EPICOPerfPackFoveationMode::Off;
	Settings.FoveationLevel = EPICOPerfPackFoveationLevel::Low;
	Settings.bSuperResolution = false;
	Settings.SharpeningSetting = ESharpeningTypePICO::None;
	Settings.SharpeningEnhanceModeSetting = ESharpeningEnhanceModePICO::None;
	Settings.bDynamicResolution = false;
	Settings.AdaptiveResolutionSetting = EAdaptiveResolutionSettingPICO::Balanced;
	Settings.bTonemapSubpass = false;
	return Settings;
}

void UPICO_PerformancePackSettings::ResetSwanToDefault()
{
	const FProperty* SwanProperty = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, Swan));
	if (!SwanProperty)
	{
		return;
	}

	const FString DefaultConfigFilename = GetDefaultConfigFilename();
	Swan = PICOPerfPackMakeSwanPreset();
	UpdateSinglePropertyInConfigFile(SwanProperty, DefaultConfigFilename);
}

void UPICO_PerformancePackSettings::ResetPICO4ToDefault()
{
	const FProperty* PICO4Property = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, PICO4));
	if (!PICO4Property)
	{
		return;
	}

	const FString DefaultConfigFilename = GetDefaultConfigFilename();
	PICO4 = PICOPerfPackMakePICO4Preset();
	UpdateSinglePropertyInConfigFile(PICO4Property, DefaultConfigFilename);
}

void UPICO_PerformancePackSettings::ResetOthersToDefault()
{
	const FProperty* OthersProperty = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, Others));
	if (!OthersProperty)
	{
		return;
	}

	const FString DefaultConfigFilename = GetDefaultConfigFilename();
	Others = PICOPerfPackMakeOthersPreset();
	UpdateSinglePropertyInConfigFile(OthersProperty, DefaultConfigFilename);
}

void UPICO_PerformancePackSettings::ApplySwan()
{
	if (IsBasicProfileEnabled(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, Swan)))
	{
		WriteDefaultDeviceProfile(TEXT("Swan"), Swan);
	}
	else
	{
		RemoveDefaultDeviceProfile(TEXT("Swan"));
	}
}

void UPICO_PerformancePackSettings::ApplyPICO4()
{
	if (IsBasicProfileEnabled(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, PICO4)))
	{
		WriteDefaultDeviceProfile(TEXT("PICO4"), PICO4);
	}
	else
	{
		RemoveDefaultDeviceProfile(TEXT("PICO4"));
	}
}

void UPICO_PerformancePackSettings::ApplyOthers()
{
	if (IsBasicProfileEnabled(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, Others)))
	{
		WriteDefaultDeviceProfile(TEXT("Others"), Others);
	}
	else
	{
		RemoveDefaultDeviceProfile(TEXT("Others"));
	}
}

bool UPICO_PerformancePackSettings::IsBasicProfileEnabled(FName ProfilePropertyName) const
{
	if (ProfilePropertyName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, Swan))
	{
		return bEnableSwanProfile;
	}
	if (ProfilePropertyName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, PICO4))
	{
		return bEnablePICO4Profile;
	}
	if (ProfilePropertyName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, Others))
	{
		return bEnableOthersProfile;
	}
	return false;
}

void UPICO_PerformancePackSettings::SetBasicProfileEnabled(FName ProfilePropertyName, bool bEnabled)
{
	FName FlagPropertyName = NAME_None;
	bool* FlagPtr = nullptr;

	if (ProfilePropertyName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, Swan))
	{
		FlagPropertyName = GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, bEnableSwanProfile);
		FlagPtr = &bEnableSwanProfile;
	}
	else if (ProfilePropertyName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, PICO4))
	{
		FlagPropertyName = GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, bEnablePICO4Profile);
		FlagPtr = &bEnablePICO4Profile;
	}
	else if (ProfilePropertyName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, Others))
	{
		FlagPropertyName = GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, bEnableOthersProfile);
		FlagPtr = &bEnableOthersProfile;
	}

	if (!FlagPtr || (FlagPtr && *FlagPtr == bEnabled))
	{
		return;
	}

	*FlagPtr = bEnabled;

	if (const FProperty* FlagProperty = GetClass()->FindPropertyByName(FlagPropertyName))
	{
		UpdateSinglePropertyInConfigFile(FlagProperty, GetDefaultConfigFilename());
	}

	if (ProfilePropertyName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, Swan))
	{
		ApplySwan();
	}
	else if (ProfilePropertyName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, PICO4))
	{
		ApplyPICO4();
	}
	else if (ProfilePropertyName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, Others))
	{
		ApplyOthers();
	}
}

void UPICO_PerformancePackSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);
	Super::TryUpdateDefaultConfigFile();

	if (PropertyChangedEvent.Property && !bEnforcingAdvancedConstraints && !bSyncingFromRendererSettings)
	{
		TGuardValue<bool> Guard(bEnforcingAdvancedConstraints, true);
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, MobileShadingPath))
		{
			PICOXR_UPDATE_SETTINGS(URendererSettings, MobileShadingPath, MobileShadingPath);
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, MobileAntiAliasing))
		{
			if (MobileAntiAliasing == EMobileAntiAliasingMethod::TemporalAA && !bMobilePostProcessing)
			{
				bMobilePostProcessing = true;
				const FProperty* MobileHDRProperty = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, bMobilePostProcessing));
				if (MobileHDRProperty)
				{
					UpdateSinglePropertyInConfigFile(MobileHDRProperty, GetDefaultConfigFilename());
				}
				PICOXR_UPDATE_SETTINGS(URendererSettings, bMobilePostProcessing, bMobilePostProcessing);
			}
			PICOXR_UPDATE_SETTINGS(URendererSettings, MobileAntiAliasing, MobileAntiAliasing);
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, MSAASampleCount))
		{
			PICOXR_UPDATE_SETTINGS(URendererSettings, MSAASampleCount, MSAASampleCount);
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, bMobilePostProcessing))
		{
			if (!bMobilePostProcessing && MobileAntiAliasing == EMobileAntiAliasingMethod::TemporalAA)
			{
				MobileAntiAliasing = EMobileAntiAliasingMethod::MSAA;
				const FProperty* MobileAAProperty = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, MobileAntiAliasing));
				if (MobileAAProperty)
				{
					UpdateSinglePropertyInConfigFile(MobileAAProperty, GetDefaultConfigFilename());
				}
				PICOXR_UPDATE_SETTINGS(URendererSettings, MobileAntiAliasing, MobileAntiAliasing);
			}
			PICOXR_UPDATE_SETTINGS(URendererSettings, bMobilePostProcessing, bMobilePostProcessing);
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, bMobileAllowMovableSpotlightShadows))
		{
			PICOXR_UPDATE_SETTINGS(URendererSettings, bMobileAllowMovableSpotlightShadows, bMobileAllowMovableSpotlightShadows);
		}
	}

	if (PropertyChangedEvent.MemberProperty)
	{
		const FName MemberName = PropertyChangedEvent.MemberProperty->GetFName();
		if (MemberName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, Swan))
		{
			ApplySwan();
		}
		else if (MemberName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, PICO4))
		{
			ApplyPICO4();
		}
		else if (MemberName == GET_MEMBER_NAME_CHECKED(UPICO_PerformancePackSettings, Others))
		{
			ApplyOthers();
		}
	}
}

bool UPICO_PerformancePackSettings::WriteDefaultDeviceProfilesIni() const
{
	const FString OutputDir = FPaths::ProjectConfigDir();
	IFileManager::Get().MakeDirectory(*OutputDir, true);
	const FString OutputPath = OutputDir / TEXT("DefaultDeviceProfiles.ini");

	auto AppendDeviceProfile = [&](TArray<FString>& Lines, const FString& ProfileName, const FPICOPerfPackDeviceSettings& Config)
	{
		if (Lines.Num() > 0)
		{
			Lines.Add(TEXT(""));
		}

		Lines.Add(FString::Printf(TEXT("[%s DeviceProfile]"), *ProfileName));
		Lines.Add(TEXT("DeviceType=Android"));
		Lines.Add(TEXT("BaseProfileName=PICO"));

		const float ResolutionScale = FMath::Clamp(Config.ResolutionScale, 0.0f, 2.0f);
		const int32 HMDRenderTargetScale = FMath::Clamp(FMath::RoundToInt(ResolutionScale * 100.0f), 0, 200);

		Lines.Add(FString::Printf(TEXT("+CVars=xr.SecondaryScreenPercentage.HMDRenderTarget=%d"), HMDRenderTargetScale));
		Lines.Add(FString::Printf(TEXT("+CVars=r.Mobile.PICO.DisplayRefreshRate=%d"), FMath::Clamp(static_cast<int32>(Config.RefreshRate), 0, 2)));
		Lines.Add(FString::Printf(TEXT("+CVars=r.Mobile.PICO.EnableSuperResolution=%d"), Config.bSuperResolution ? 1 : 0));
		Lines.Add(FString::Printf(TEXT("+CVars=r.Mobile.PICO.EnableDynamicResolution=%d"), Config.bDynamicResolution ? 1 : 0));
		Lines.Add(FString::Printf(TEXT("+CVars=r.Mobile.PICO.AdaptiveResolutionSetting=%d"), static_cast<int32>(Config.AdaptiveResolutionSetting)));
		Lines.Add(FString::Printf(TEXT("+CVars=r.Mobile.PICO.SharpeningSetting=%d"), static_cast<int32>(Config.SharpeningSetting)));
		Lines.Add(FString::Printf(TEXT("+CVars=r.Mobile.PICO.SharpeningEnhanceMode=%d"), static_cast<int32>(Config.SharpeningEnhanceModeSetting)));
#ifdef PICO_CUSTOM_ENGINE
		const bool bFBFoveation = (Config.FoveationMode == EPICOPerfPackFoveationMode::Fixed) || (Config.FoveationMode == EPICOPerfPackFoveationMode::EyeTracked);
		const bool bFBFoveationEyeTracked = (Config.FoveationMode == EPICOPerfPackFoveationMode::EyeTracked);
#else
		const bool bFBFoveation = Config.FoveationMode == EPICOPerfPackFoveationMode::Fixed;
		const bool bFBFoveationEyeTracked = false;
#endif
		
		if (bFBFoveation)
		{
			Lines.Add(TEXT("+CVars=r.VRS.Enable=1"));
		}
		Lines.Add(FString::Printf(TEXT("+CVars=xr.OpenXRFBFoveation=%d"), bFBFoveation ? 1 : 0));
		Lines.Add(FString::Printf(TEXT("+CVars=xr.OpenXRFBFoveationEyetracked=%d"), bFBFoveationEyeTracked ? 1 : 0));
		const int32 FoveationLevel = (Config.FoveationMode == EPICOPerfPackFoveationMode::Off) ? 0 : static_cast<int32>(Config.FoveationLevel);
		Lines.Add(FString::Printf(TEXT("+CVars=xr.OpenXRFBFoveationLevel=%d"), FMath::Clamp(FoveationLevel, 0, 3)));

		if (Config.bTonemapSubpass)
		{
			Lines.Add(TEXT("+CVars=r.Mobile.TonemapSubpass=1"));
		}
	};

	TArray<FString> Lines;
	AppendDeviceProfile(Lines, TEXT("Swan"), Swan);
	AppendDeviceProfile(Lines, TEXT("PICO4"), PICO4);
	AppendDeviceProfile(Lines, TEXT("Others"), Others);
	return FFileHelper::SaveStringArrayToFile(Lines, *OutputPath);
}

static void PICOPerfPackReplaceIniSection(TArray<FString>& Lines, const FString& SectionHeader, const TArray<FString>& NewSectionLines)
{
	auto IsSectionLine = [](const FString& Line)
	{
		return Line.StartsWith(TEXT("[")) && Line.EndsWith(TEXT("]"));
	};

	const int32 ExistingStart = Lines.IndexOfByKey(SectionHeader);
	if (ExistingStart != INDEX_NONE)
	{
		int32 ExistingEnd = ExistingStart + 1;
		while (ExistingEnd < Lines.Num() && !IsSectionLine(Lines[ExistingEnd]))
		{
			ExistingEnd++;
		}
		Lines.RemoveAt(ExistingStart, ExistingEnd - ExistingStart);

		for (int32 Index = NewSectionLines.Num() - 1; Index >= 0; --Index)
		{
			Lines.Insert(NewSectionLines[Index], ExistingStart);
		}
		return;
	}

	if (Lines.Num() > 0 && !Lines.Last().IsEmpty())
	{
		Lines.Add(TEXT(""));
	}
	Lines.Append(NewSectionLines);
}

bool UPICO_PerformancePackSettings::WriteDefaultDeviceProfile(const FString& ProfileName, const FPICOPerfPackDeviceSettings& Config) const
{
	const FString OutputDir = FPaths::ProjectConfigDir();
	IFileManager::Get().MakeDirectory(*OutputDir, true);
	const FString OutputPath = OutputDir / TEXT("DefaultDeviceProfiles.ini");

	TArray<FString> Lines;
	if (IFileManager::Get().FileExists(*OutputPath))
	{
		FFileHelper::LoadFileToStringArray(Lines, *OutputPath);
	}

	const FString SectionHeader = FString::Printf(TEXT("[%s DeviceProfile]"), *ProfileName);

	const float ResolutionScale = FMath::Clamp(Config.ResolutionScale, 0.0f, 2.0f);
	const int32 HMDRenderTargetScale = FMath::Clamp(FMath::RoundToInt(ResolutionScale * 100.0f), 0, 200);

	TArray<FString> NewSectionLines;
	NewSectionLines.Add(SectionHeader);
	NewSectionLines.Add(TEXT("DeviceType=Android"));
	NewSectionLines.Add(TEXT("BaseProfileName=PICO"));
	NewSectionLines.Add(FString::Printf(TEXT("+CVars=xr.SecondaryScreenPercentage.HMDRenderTarget=%d"), HMDRenderTargetScale));
	NewSectionLines.Add(FString::Printf(TEXT("+CVars=r.Mobile.PICO.DisplayRefreshRate=%d"), FMath::Clamp(static_cast<int32>(Config.RefreshRate), 0, 2)));
	NewSectionLines.Add(FString::Printf(TEXT("+CVars=r.Mobile.PICO.EnableSuperResolution=%d"), Config.bSuperResolution ? 1 : 0));
	NewSectionLines.Add(FString::Printf(TEXT("+CVars=r.Mobile.PICO.EnableDynamicResolution=%d"), Config.bDynamicResolution ? 1 : 0));
	NewSectionLines.Add(FString::Printf(TEXT("+CVars=r.Mobile.PICO.AdaptiveResolutionSetting=%d"), static_cast<int32>(Config.AdaptiveResolutionSetting)));
	NewSectionLines.Add(FString::Printf(TEXT("+CVars=r.Mobile.PICO.SharpeningSetting=%d"), static_cast<int32>(Config.SharpeningSetting)));
	NewSectionLines.Add(FString::Printf(TEXT("+CVars=r.Mobile.PICO.SharpeningEnhanceMode=%d"), static_cast<int32>(Config.SharpeningEnhanceModeSetting)));

#ifdef PICO_CUSTOM_ENGINE
	const bool bFBFoveation = (Config.FoveationMode == EPICOPerfPackFoveationMode::Fixed) || (Config.FoveationMode == EPICOPerfPackFoveationMode::EyeTracked);
	const bool bFBFoveationEyeTracked = (Config.FoveationMode == EPICOPerfPackFoveationMode::EyeTracked);
#else
	const bool bFBFoveation = Config.FoveationMode == EPICOPerfPackFoveationMode::Fixed;
	const bool bFBFoveationEyeTracked = false;
#endif	
	
	if (bFBFoveation)
	{
		NewSectionLines.Add(TEXT("+CVars=r.VRS.Enable=1"));
	}
	NewSectionLines.Add(FString::Printf(TEXT("+CVars=xr.OpenXRFBFoveation=%d"), bFBFoveation ? 1 : 0));
	NewSectionLines.Add(FString::Printf(TEXT("+CVars=xr.OpenXRFBFoveationEyetracked=%d"), bFBFoveationEyeTracked ? 1 : 0));
	const int32 FoveationLevel = (Config.FoveationMode == EPICOPerfPackFoveationMode::Off) ? 0 : static_cast<int32>(Config.FoveationLevel);
	NewSectionLines.Add(FString::Printf(TEXT("+CVars=xr.OpenXRFBFoveationLevel=%d"), FMath::Clamp(FoveationLevel, 0, 3)));

	if (Config.bTonemapSubpass)
	{
		NewSectionLines.Add(TEXT("+CVars=r.Mobile.TonemapSubpass=1"));
	}

	PICOPerfPackReplaceIniSection(Lines, SectionHeader, NewSectionLines);
	return FFileHelper::SaveStringArrayToFile(Lines, *OutputPath);
}

bool UPICO_PerformancePackSettings::RemoveDefaultDeviceProfile(const FString& ProfileName) const
{
	const FString OutputPath = FPaths::ProjectConfigDir() / TEXT("DefaultDeviceProfiles.ini");
	if (!IFileManager::Get().FileExists(*OutputPath))
	{
		return true;
	}

	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *OutputPath))
	{
		return false;
	}

	auto IsSectionLine = [](const FString& Line)
	{
		return Line.StartsWith(TEXT("[")) && Line.EndsWith(TEXT("]"));
	};

	const FString SectionHeader = FString::Printf(TEXT("[%s DeviceProfile]"), *ProfileName);
	const int32 ExistingStart = Lines.IndexOfByKey(SectionHeader);
	if (ExistingStart == INDEX_NONE)
	{
		return true;
	}

	int32 ExistingEnd = ExistingStart + 1;
	while (ExistingEnd < Lines.Num() && !IsSectionLine(Lines[ExistingEnd]))
	{
		ExistingEnd++;
	}

	int32 RemoveStart = ExistingStart;
	if (RemoveStart > 0 && Lines[RemoveStart - 1].IsEmpty())
	{
		RemoveStart--;
	}

	Lines.RemoveAt(RemoveStart, ExistingEnd - RemoveStart);
	return FFileHelper::SaveStringArrayToFile(Lines, *OutputPath);
}

bool UPICO_PerformancePackSettings::SyncDefaultDeviceProfilesFromToggles() const
{
	const bool bSwanOk = bEnableSwanProfile ? WriteDefaultDeviceProfile(TEXT("Swan"), Swan) : RemoveDefaultDeviceProfile(TEXT("Swan"));
	const bool bPico4Ok = bEnablePICO4Profile ? WriteDefaultDeviceProfile(TEXT("PICO4"), PICO4) : RemoveDefaultDeviceProfile(TEXT("PICO4"));
	const bool bOthersOk = bEnableOthersProfile ? WriteDefaultDeviceProfile(TEXT("Others"), Others) : RemoveDefaultDeviceProfile(TEXT("Others"));
	return bSwanOk && bPico4Ok && bOthersOk;
}

static bool PICOPerfPackEnsureMatchProfileLine(TArray<FString>& Lines, int32 InsertIndex, const FString& NewLine)
{
	for (const FString& Existing : Lines)
	{
		if (Existing == NewLine)
		{
			return false;
		}
	}
	Lines.Insert(NewLine, InsertIndex);
	return true;
}

static void PICOPerfPackEnsureLineInSection(TArray<FString>& Lines, const FString& SectionHeader, const FString& LineToEnsure)
{
	int32 SectionIndex = Lines.IndexOfByKey(SectionHeader);
	if (SectionIndex == INDEX_NONE)
	{
		Lines.Add(TEXT(""));
		Lines.Add(SectionHeader);
		SectionIndex = Lines.Num() - 1;
	}

	int32 InsertIndex = SectionIndex + 1;
	while (InsertIndex < Lines.Num())
	{
		const FString& Line = Lines[InsertIndex];
		if (Line.StartsWith(TEXT("[")) && Line.EndsWith(TEXT("]")))
		{
			break;
		}
		InsertIndex++;
	}

	for (int32 LineIndex = SectionIndex + 1; LineIndex < InsertIndex; LineIndex++)
	{
		if (Lines[LineIndex] == LineToEnsure)
		{
			return;
		}
	}

	Lines.Insert(LineToEnsure, InsertIndex);
}

bool UPICO_PerformancePackSettings::WriteBaseDeviceProfilesMatchProfiles() const
{
	const FString EngineConfigDir = FPaths::EngineConfigDir();
	const FString BaseDeviceProfilesPath = EngineConfigDir / TEXT("BaseDeviceProfiles.ini");

	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *BaseDeviceProfilesPath))
	{
		return false;
	}
	const TArray<FString> OriginalLines = Lines;

	PICOPerfPackEnsureLineInSection(Lines, TEXT("[DeviceProfiles]"), TEXT("+DeviceProfileNameAndTypes=PICO,Android"));
	PICOPerfPackEnsureLineInSection(Lines, TEXT("[DeviceProfiles]"), TEXT("+DeviceProfileNameAndTypes=Swan,Android"));
	PICOPerfPackEnsureLineInSection(Lines, TEXT("[DeviceProfiles]"), TEXT("+DeviceProfileNameAndTypes=PICO4,Android"));
	PICOPerfPackEnsureLineInSection(Lines, TEXT("[DeviceProfiles]"), TEXT("+DeviceProfileNameAndTypes=Others,Android"));

	{
		TArray<FString> PicoSectionLines;
		PicoSectionLines.Add(TEXT("[PICO DeviceProfile]"));
		PicoSectionLines.Add(TEXT("DeviceType=Android"));
		PicoSectionLines.Add(TEXT("BaseProfileName=Android_OpenXR"));
		PicoSectionLines.Add(TEXT("+CVars=r.Android.DisableVulkanSupport=0"));
		PicoSectionLines.Add(TEXT("+CVars=r.Vulkan.VRSFormat=3"));
		PICOPerfPackReplaceIniSection(Lines, TEXT("[PICO DeviceProfile]"), PicoSectionLines);
	}

	const FString SectionHeader = TEXT("[/Script/AndroidDeviceProfileSelector.AndroidDeviceProfileMatchingRules]");
	int32 SectionIndex = Lines.IndexOfByKey(SectionHeader);
	if (SectionIndex == INDEX_NONE)
	{
		Lines.Add(TEXT(""));
		Lines.Add(SectionHeader);
		SectionIndex = Lines.Num() - 1;
	}

	int32 InsertIndex = SectionIndex + 1;
	while (InsertIndex < Lines.Num())
	{
		const FString& Line = Lines[InsertIndex];
		if (Line.StartsWith(TEXT("[")) && Line.EndsWith(TEXT("]")))
		{
			break;
		}
		InsertIndex++;
	}

	const FString PICO4Match = TEXT("+MatchProfile=(Profile=\"PICO4\",Match=((SourceType=SRC_DeviceMake,CompareType=CMP_EqualIgnore,MatchString=\"pico\"), (SourceType=SRC_HMDSystemName,CompareType=CMP_EqualIgnore,MatchString=\"PICO 4 Ultra HMD\")))");
	const FString SwanMatch = TEXT("+MatchProfile=(Profile=\"Swan\",Match=((SourceType=SRC_DeviceMake,CompareType=CMP_EqualIgnore,MatchString=\"pico\"), (SourceType=SRC_HMDSystemName,CompareType=CMP_EqualIgnore,MatchString=\"PICO HMD\")))");
	const FString OthersMatch = TEXT("+MatchProfile=(Profile=\"Others\",Match=((SourceType=SRC_DeviceMake,CompareType=CMP_EqualIgnore,MatchString=\"pico\"), (SourceType=SRC_HMDSystemName,CompareType=CMP_NotEqualIgnore,MatchString=\"PICO 4 Ultra HMD\"), (SourceType=SRC_HMDSystemName,CompareType=CMP_NotEqualIgnore,MatchString=\"PICO HMD\")))");

	for (int32 LineIndex = InsertIndex - 1; LineIndex > SectionIndex; LineIndex--)
	{
		if (Lines[LineIndex] == PICO4Match || Lines[LineIndex] == SwanMatch || Lines[LineIndex] == OthersMatch)
		{
			Lines.RemoveAt(LineIndex);
		}
	}

	int32 MagicLeapIndex = INDEX_NONE;
	for (int32 LineIndex = SectionIndex + 1; LineIndex < InsertIndex; LineIndex++)
	{
		if (Lines[LineIndex].Contains(TEXT("Profile=\"MagicLeap_Vulkan\"")))
		{
			MagicLeapIndex = LineIndex;
			break;
		}
	}

	const int32 DesiredInsertIndex = (MagicLeapIndex != INDEX_NONE) ? (MagicLeapIndex + 1) : (SectionIndex + 1);
	int32 DesiredIndex = DesiredInsertIndex;
	if (PICOPerfPackEnsureMatchProfileLine(Lines, DesiredIndex, PICO4Match))
	{
		DesiredIndex++;
	}
	if (PICOPerfPackEnsureMatchProfileLine(Lines, DesiredIndex, SwanMatch))
	{
		DesiredIndex++;
	}
	PICOPerfPackEnsureMatchProfileLine(Lines, DesiredIndex, OthersMatch);

	const FString BackupPath = BaseDeviceProfilesPath + TEXT(".PerformancePack.bak");
	if (!IFileManager::Get().FileExists(*BackupPath))
	{
		FFileHelper::SaveStringArrayToFile(OriginalLines, *BackupPath);
	}

	return FFileHelper::SaveStringArrayToFile(Lines, *BaseDeviceProfilesPath);
}
