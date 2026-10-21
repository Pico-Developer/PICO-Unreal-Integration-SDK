// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialRuntimeSettings.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/FileManager.h"
#include "Engine/RendererSettings.h"
#include "Misc/ConfigUtilities.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY(LogPICOSpatialRuntimeSettings);

IMPLEMENT_MODULE(FDefaultModuleImpl, PICOSpatialRuntimeSettings);

// Console variable for controlling USD file format
// True = use binary .usd format, False = use ASCII .usda format
static TAutoConsoleVariable<bool> CVarUseUsdBinaryFormat(
	TEXT("PICO.UseUsdBinaryFormat"),
	true,
	TEXT("Controls the USD file format for PICO Spatial asset export.\n")
	TEXT("  0: Use ASCII .usda format (more readable, larger file size)\n")
	TEXT("  1: Use binary .usd format (faster loading, smaller file size) [Default]"),
	ECVF_Default
);

static TAutoConsoleVariable<bool> CVarKeepBackgroundContainerEntityProxiesAlive(
	TEXT("PICO.KeepBackgroundContainerEntityProxiesAlive"),
	false,
	TEXT("Controls whether background spatial containers keep their existing entity proxies alive.\n")
	TEXT("  0: Release background container proxies through dead-proxy cleanup [Default]\n")
	TEXT("  1: Keep previous proxies alive while backgrounded until timeout"),
	ECVF_Default
);

UPICOSpatialRuntimeSettings::UPICOSpatialRuntimeSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UE::ConfigUtilities::ApplyCVarSettingsFromIni(TEXT("/Script/PICOSpatialRuntimeSettings.PICOSpatialRuntimeSettings"), *GEngineIni, ECVF_SetByProjectSetting);
}

void UPICOSpatialRuntimeSettings::PostInitProperties()
{
	Super::PostInitProperties();
#if WITH_EDITOR
	FPropertyChangedEvent PropertyChangedEvent{ nullptr };
	PostEditChangeProperty(PropertyChangedEvent);
#endif
}

void UPICOSpatialRuntimeSettings::HandlePICOSpatialSupport()
{
	if (GConfig == nullptr)
	{
		return;
	}

	const FString DefaultEnginePath = FConfigCacheIni::NormalizeConfigIniPath(FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("DefaultEngine.ini")));
	GConfig->LoadFile(DefaultEnginePath);

	// Configure Android build settings for PICO Spatial
	GConfig->SetBool(TEXT("/Script/AndroidRuntimeSettings.AndroidRuntimeSettings"), TEXT("bBuildForX8664"), false, DefaultEnginePath);
	GConfig->SetBool(TEXT("/Script/AndroidRuntimeSettings.AndroidRuntimeSettings"), TEXT("bBuildForArm64"), true, DefaultEnginePath);

	// Build extra activity settings based on spatial container type
	FString ExtraActivitySettings;
	switch (SpatialContainerTypePICO)
	{
	case ESpatialContainerTypePICO::Default2D:
	{
		ExtraActivitySettings = FString("<meta-data android:name=\"pvr.app.type\" android:value=\"2d\" />");
		break;
	}
	case ESpatialContainerTypePICO::Bounded:
	{
		// Calculate size string from container dimensions (convert from cm to PICO units)
		const int32 SizeX = static_cast<int32>(SpatialContainerSize.Y * 25.0f);
		const int32 SizeY = static_cast<int32>(SpatialContainerSize.Z * 25.0f);
		const int32 SizeZ = static_cast<int32>(SpatialContainerSize.X * 25.0f);
		const FString SpatialContainerSizeString = FString::Printf(TEXT("%dx%dx%d"), SizeX, SizeY, SizeZ);
		const FString AutoHideCaptionBarValue = bAutoHideCaptionBar ? TEXT("1") : TEXT("0");
		
		ExtraActivitySettings = FString::Printf(TEXT("<meta-data android:name=\"pico.spatial.windowcontainer.id\" android:value=\"%s\" />"), *SpatialContainerName);
		ExtraActivitySettings += TEXT("\\n<meta-data android:name=\"pico.spatial.windowcontainer.style\" android:value=\"2\" />");
		ExtraActivitySettings += FString::Printf(TEXT("\\n<meta-data android:name=\"pico.spatial.windowcontainer.size\" android:value=\"%s\" />"), *SpatialContainerSizeString);
		ExtraActivitySettings += FString::Printf(TEXT("\\n<meta-data android:name=\"pico.spatial.windowcontainer.maxsize\" android:value=\"%s\" />"), *SpatialContainerSizeString);
		ExtraActivitySettings += FString::Printf(TEXT("\\n<meta-data android:name=\"pico.spatial.windowcontainer.minsize\" android:value=\"%s\" />"), *SpatialContainerSizeString);
		ExtraActivitySettings += FString::Printf(TEXT("\\n<meta-data android:name=\"pico.spatial.windowcontainer.captionbar\" android:value=\"%s\" />"), *AutoHideCaptionBarValue);
		ExtraActivitySettings += TEXT("\\n<meta-data android:name=\"pico.spatial.windowcontainer.worldscaletype\" android:value=\"2\" />");
		break;
	}
	case ESpatialContainerTypePICO::Unbounded:
	{
		const int32 StyleValue = static_cast<int32>(UnboundedStylePICO);
		ExtraActivitySettings = FString::Printf(TEXT("<meta-data android:name=\"pico.spatial.stage.id\" android:value=\"%s\" />"), *SpatialContainerName);
		ExtraActivitySettings += FString::Printf(TEXT("\\n<meta-data android:name=\"pico.spatial.stage.style\" android:value=\"%d\" />"), StyleValue);
		break;
	}
	default:
		break;
	}
	
	GConfig->SetString(TEXT("/Script/AndroidRuntimeSettings.AndroidRuntimeSettings"), TEXT("ExtraActivitySettings"), *ExtraActivitySettings, DefaultEnginePath);

	GConfig->Flush(false);
	GConfig->UnloadFile(DefaultEnginePath);
}

#if WITH_EDITOR
void UPICOSpatialRuntimeSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	Super::TryUpdateDefaultConfigFile();

	// Sync bUseUsdBinaryFormat property change to console variable
	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UPICOSpatialRuntimeSettings, bUseUsdBinaryFormat))
		{
			// Update the console variable to match the property value
			IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("PICO.UseUsdBinaryFormat"));
			if (CVar)
			{
				CVar->Set(bUseUsdBinaryFormat, ECVF_SetByProjectSetting);
				UE_LOG(LogPICOSpatialRuntimeSettings, Log, 
					TEXT("Updated PICO.UseUsdBinaryFormat console variable to: %s"), 
					bUseUsdBinaryFormat ? TEXT("true (.usd)") : TEXT("false (.usda)"));
			}
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UPICOSpatialRuntimeSettings, bKeepBackgroundContainerEntityProxiesAlive))
		{
			IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("PICO.KeepBackgroundContainerEntityProxiesAlive"));
			if (CVar)
			{
				CVar->Set(bKeepBackgroundContainerEntityProxiesAlive, ECVF_SetByProjectSetting);
				UE_LOG(LogPICOSpatialRuntimeSettings, Log,
					TEXT("Updated PICO.KeepBackgroundContainerEntityProxiesAlive console variable to: %s"),
					bKeepBackgroundContainerEntityProxiesAlive ? TEXT("true") : TEXT("false"));
			}
		}
	}

	// Handle reset of UseDefaultWindow setting
	if (bResetCustomDefaultWindow)
	{
		bCustomDefaultWindow = bLastCustomDefaultWindow;
		UpdateSinglePropertyInConfigFile(GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICOSpatialRuntimeSettings, bCustomDefaultWindow)), GetDefaultConfigFilename());
		bResetCustomDefaultWindow = false;
	}

	// Update UseDefaultWindow based on container type
	if (SpatialContainerTypePICO <= ESpatialContainerTypePICO::Bounded)
	{
		bLastCustomDefaultWindow = bCustomDefaultWindow;
	}
	else
	{
		bResetCustomDefaultWindow = true;
		bCustomDefaultWindow = false;
		UpdateSinglePropertyInConfigFile(GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICOSpatialRuntimeSettings, bCustomDefaultWindow)), GetDefaultConfigFilename());
	}

	// Apply spatial configuration
	HandlePICOSpatialSupport();
}
#endif // WITH_EDITOR
