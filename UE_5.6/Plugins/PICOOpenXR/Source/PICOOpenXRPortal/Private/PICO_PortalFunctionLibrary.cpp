// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_PortalFunctionLibrary.h"
#include "ISettingsModule.h"
#include "PICOOpenXRRuntimeSettings.h"
#include "PICO_AppLog.h"
#include "PICO_PerformancePackSettings.h"
#include "PICO_PortalSubsystem.h"
#include "PICO_PortalSettings.h"
#include "Interfaces/IPluginManager.h"

bool UPICOPortalFunctionLibrary::bShowRestartEditorButton = false;
FString UPICOPortalFunctionLibrary::XRPortal = "PICOXRPortal";
FString UPICOPortalFunctionLibrary::XRToolKit = "PICOXRToolKit";
FString UPICOPortalFunctionLibrary::XRSettings = "PICOXRSettings";
FString UPICOPortalFunctionLibrary::XREngineVersion = "EngineVersion";
FString UPICOPortalFunctionLibrary::XRSDKVersion = "SDKVersion";
FString UPICOPortalFunctionLibrary::SDKType = "UnrealSDKType";
FString UPICOPortalFunctionLibrary::EngineType = "UnrealEngineType";
FString UPICOPortalFunctionLibrary::PICOXRSDK = "PICOXRSDK";
FString UPICOPortalFunctionLibrary::OpenXRSDK = "OpenXRSDK";
FString UPICOPortalFunctionLibrary::OfficialEngine = "OfficialEngine";
FString UPICOPortalFunctionLibrary::PICOCustomEngine = "PICOCustomEngine";

FString UPICOPortalFunctionLibrary::XRPortalStartUp = "PICOXRPortalStartUp";
FString UPICOPortalFunctionLibrary::ProjectVerificationStartUp = "ProjectVerificationStartUp";
FString UPICOPortalFunctionLibrary::XRPICOSettingsStartUp = "PICOXRPICOSettingsStartUp";
FString UPICOPortalFunctionLibrary::PICODeveloperCenter = "PICODeveloperCenter";
FString UPICOPortalFunctionLibrary::PICOEmulator = "PICOEmulator";
FString UPICOPortalFunctionLibrary::GetStartedSample = "GetStartedSample";
FString UPICOPortalFunctionLibrary::PlatformSample = "PlatformSample";
FString UPICOPortalFunctionLibrary::SpatialAudioSample = "SpatialAudioSample";
FString UPICOPortalFunctionLibrary::MRSample = "MRSample";
FString UPICOPortalFunctionLibrary::HandTrackingSample = "HandTrackingSample";
FString UPICOPortalFunctionLibrary::ControllerInteraction = "ControllerInteraction";
FString UPICOPortalFunctionLibrary::BodyTracking = "BodyTracking";
FString UPICOPortalFunctionLibrary::PICODeveloperWebsite = "PICODeveloperWebsite";
FString UPICOPortalFunctionLibrary::DownloadFromPICO_Official = "DownloadFromPICO_Official";
FString UPICOPortalFunctionLibrary::DownloadFromUnreal_Fab = "DownloadFromUnreal_Fab";
FString UPICOPortalFunctionLibrary::DownloadPICOForkOfUnrealEngine = "DownloadPICOForkOfUnrealEngine";

FString UPICOPortalFunctionLibrary::SpatialMeshActor = "SpatialMeshActor";
FString UPICOPortalFunctionLibrary::SpatialAnchorActor = "SpatialAnchorActor";
FString UPICOPortalFunctionLibrary::SceneCapturesGenerator = "SceneCapturesGenerator";
FString UPICOPortalFunctionLibrary::PICOXRPawn = "PICOXRPawn";
FString UPICOPortalFunctionLibrary::HandTrackingComponent = "HandTrackingComponent";
FString UPICOPortalFunctionLibrary::EyeTrackingComponent = "EyeTrackingComponent";
FString UPICOPortalFunctionLibrary::FaceTrackingComponent = "FaceTrackingComponent";

FString UPICOPortalFunctionLibrary::MobileHDR = "MobileHDR";
FString UPICOPortalFunctionLibrary::DeferredShading = "DeferredShading";
FString UPICOPortalFunctionLibrary::ForwardShading = "ForwardShading";
FString UPICOPortalFunctionLibrary::LateLatching = "LateLatching";
FString UPICOPortalFunctionLibrary::MovableSpotlight = "MovableSpotlight";
FString UPICOPortalFunctionLibrary::ApplicationSpaceWarp = "ApplicationSpaceWarp";
FString UPICOPortalFunctionLibrary::ToneMapping = "ToneMapping";
FString UPICOPortalFunctionLibrary::FXAA = "FXAA";
FString UPICOPortalFunctionLibrary::TAA = "TAA";
FString UPICOPortalFunctionLibrary::MSAA = "MSAA";


TArray<UPICOVerificationObject*> UPICOPortalFunctionLibrary::GetFixedVerificationObjects()
{
	UPICO_PortalSubsystem* ProcessorSubsystem = GEngine->GetEngineSubsystem<UPICO_PortalSubsystem>();
	return ProcessorSubsystem->FixedVerificationObjects;
}

TArray<UPICOVerificationObject*> UPICOPortalFunctionLibrary::GetRequiredVerificationObjects()
{
	UPICO_PortalSubsystem* ProcessorSubsystem = GEngine->GetEngineSubsystem<UPICO_PortalSubsystem>();
	return ProcessorSubsystem->RequiredVerificationObjects;
}

static UPICOVerificatonEventManager* EventManagerInstance = nullptr;


UPICOVerificatonEventManager* UPICOPortalFunctionLibrary::PXR_GetVerificationEventManager()
{
	return UPICOVerificatonEventManager::GetInstance();
}

bool UPICOPortalFunctionLibrary::HasVerificationNeedRestart()
{
	return bShowRestartEditorButton;
}

void UPICOPortalFunctionLibrary::SetVerificationNeedRestart(bool bRestart)
{
	bShowRestartEditorButton=bRestart;
}

void UPICOPortalFunctionLibrary::StartUpdatePortalTick()
{
	UPICO_PortalSubsystem* PortalSubsystem = GEngine->GetEngineSubsystem<UPICO_PortalSubsystem>();
	bool bRestartEditorButton=false;
	if(PortalSubsystem&&PortalSubsystem->Refresh(bRestartEditorButton))
	{
		UPICOVerificatonEventManager::GetInstance()->VerificatonUpdatedDelegate.Broadcast();

		if(bRestartEditorButton)
		{
			SetVerificationNeedRestart(bRestartEditorButton);
		}
	}
}

void UPICOPortalFunctionLibrary::FixAllVerifications()
{
	for (auto VerificationObject : GetRequiredVerificationObjects())
	{
		VerificationObject->PerformVerificationFix();
	}
}

UPICOVerificatonEventManager* UPICOVerificatonEventManager::GetInstance()
{
	if (EventManagerInstance == nullptr)
	{
		EventManagerInstance = NewObject<UPICOVerificatonEventManager>();
		EventManagerInstance->AddToRoot();
	}
	return EventManagerInstance;
}

bool UPICOPortalFunctionLibrary::IsShowPortalOnEditorStart()
{
	UPICO_PortalSettings* VerificationSettings=GetMutableDefault<UPICO_PortalSettings>();
	return VerificationSettings->bShowPortalOnEditorStart;
}

bool UPICOPortalFunctionLibrary::IsBehaviorDataCollectionEnabled()
{
	UPICO_PortalSettings* VerificationSettings=GetMutableDefault<UPICO_PortalSettings>();
	return VerificationSettings->bBehaviorDataCollectionEnabled;
}

void UPICOPortalFunctionLibrary::SetShowPortalOnEditorStart(bool bShowPortalOnEditorStart)
{
	UPICO_PortalSettings* VerificationSettings=GetMutableDefault<UPICO_PortalSettings>();
	VerificationSettings->bShowPortalOnEditorStart=bShowPortalOnEditorStart;

	VerificationSettings->SaveConfig();
}

void UPICOPortalFunctionLibrary::SetBehaviorDataCollectionEnabled(bool bBehaviorDataCollectionEnabled)
{
	UPICO_PortalSettings* VerificationSettings=GetMutableDefault<UPICO_PortalSettings>();
	VerificationSettings->bBehaviorDataCollectionEnabled=bBehaviorDataCollectionEnabled;

	VerificationSettings->SaveConfig();
}
#define LOCTEXT_NAMESPACE "PICOXRPortalModule"
void UPICOPortalFunctionLibrary::LaunchPICOSettings()
{
	FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer(FName("PICO"), FName("General"), FName("PICOXR Settings"));
}

void UPICOPortalFunctionLibrary::LaunchPICOPerformanceSettings()
{
	FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer(FName("PICO"), FName("General"), FName("PICOPerformancePackSettings"));
}
#undef LOCTEXT_NAMESPACE


FString UPICOPortalFunctionLibrary::GetPICOSDKVersionString()
{
	if (TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("PICOOpenXR")))
	{
		if (Plugin->IsEnabled())
		{
			return FString("Version ")+Plugin->GetDescriptor().VersionName;
		}
	}
	
	return FString("Version Unknown");
}

bool UPICOPortalFunctionLibrary::IsXRPluginVerified()
{
	for (auto VerificationObject : GetRequiredVerificationObjects())
	{
		if (VerificationObject->GetDisplayName().ToString()==FString("Disable PICO XR Plugin"))
		{
			return false;
		}
	}
	
	return true;
}

void UPICOPortalFunctionLibrary::FixXRPluginConfig()
{
	for (auto VerificationObject : GetRequiredVerificationObjects())
	{
		if (VerificationObject->GetDisplayName().ToString()==FString("Disable PICO XR Plugin"))
		{
			VerificationObject->PerformVerificationFix();
			return;
		}
	}
}

bool UPICOPortalFunctionLibrary::IsVulkanVerified()
{
	for (auto VerificationObject : GetRequiredVerificationObjects())
	{
		if (VerificationObject->GetDisplayName().ToString()==FString("Use Vulkan Rendering Backend"))
		{
			return false;
		}
	}
	
	return true;
}

void UPICOPortalFunctionLibrary::FixVulkanConfig()
{
	for (auto VerificationObject : GetRequiredVerificationObjects())
	{
		if (VerificationObject->GetDisplayName().ToString()==FString("Use Vulkan Rendering Backend"))
		{
			VerificationObject->PerformVerificationFix();
			return;
		}
	}
}

bool UPICOPortalFunctionLibrary::IsMultiviewVerified()
{
	for (auto VerificationObject : GetRequiredVerificationObjects())
	{
		if (VerificationObject->GetDisplayName().ToString()==FString("Enable Multiview"))
		{
			return false;
		}
	}
	
	return true;
}

void UPICOPortalFunctionLibrary::FixMultiviewConfig()
{
	for (auto VerificationObject : GetRequiredVerificationObjects())
	{
		if (VerificationObject->GetDisplayName().ToString()==FString("Enable Multiview"))
		{
			VerificationObject->PerformVerificationFix();
			return;
		}
	}
}

bool UPICOPortalFunctionLibrary::IsAndroidSDKVersionVerified()
{
	for (auto VerificationObject : GetRequiredVerificationObjects())
	{
		if (VerificationObject->GetDisplayName().ToString()==FString("Android SDK Minimum Version")
			||VerificationObject->GetDisplayName().ToString()==FString("Android SDK Target Version"))
		{
			return false;
		}
 	}
	
	return true;
}

void UPICOPortalFunctionLibrary::FixAndroidSDKVersionConfig()
{
	for (auto VerificationObject : GetRequiredVerificationObjects())
	{
		if (VerificationObject->GetDisplayName().ToString()==FString("Android SDK Minimum Version")
			||VerificationObject->GetDisplayName().ToString()==FString("Android SDK Target Version"))
		{
			VerificationObject->PerformVerificationFix();
		}
	}
}

void UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO AppLogType,const FString& AppLogString)
{
	if (!IsBehaviorDataCollectionEnabled())
	{
		return;
	}
	
	FString Param;
	FString Value;
	switch (AppLogType)
	{
	case EAppLogTypePICO::EngineVersion:
		Param = XREngineVersion;
		Value = AppLogString;
		break;
	case EAppLogTypePICO::SDKVersion:
		Param = XRSDKVersion;
		Value = AppLogString;
		break;
	case EAppLogTypePICO::SDKType:
		Param = SDKType;
		Value = OpenXRSDK;
		break;
	case EAppLogTypePICO::EngineType:
    	Param = EngineType;
#ifdef PICO_CUSTOM_ENGINE
		Value = PICOCustomEngine;
#else
		Value = OfficialEngine;
#endif
    	break;
	case EAppLogTypePICO::PICOXRPortalStartUp:
		Param = XRPortal;
		Value = XRPortalStartUp;
		break;
	case EAppLogTypePICO::PICOSettingsStartUp:
		Param = XRPortal;
		Value = XRPICOSettingsStartUp;
		break;
	case EAppLogTypePICO::ProjectVerificationStartUp:
		Param = XRPortal;
		Value = ProjectVerificationStartUp;
		break;
	case EAppLogTypePICO::PICODeveloperCenter:
		Param = XRPortal;
		Value = PICODeveloperCenter;
		break;
	case EAppLogTypePICO::PICOEmulator:
		Param = XRPortal;
		Value = PICOEmulator;
		break;
	case EAppLogTypePICO::GetStartedSample:
		Param = XRPortal;
		Value = GetStartedSample;
		break;
	case EAppLogTypePICO::PlatformSample:
		Param = XRPortal;
		Value = PlatformSample;
		break;
	case EAppLogTypePICO::SpatialAudioSample:
		Param = XRPortal;
		Value = SpatialAudioSample;
		break;
	case EAppLogTypePICO::MRSample:
		Param = XRPortal;
		Value = MRSample;
		break;
	case EAppLogTypePICO::HandTrackingSample:
		Param = XRPortal;
		Value = HandTrackingSample;
		break;
	case EAppLogTypePICO::ControllerInteraction:
		Param = XRPortal;
		Value = ControllerInteraction;
		break;
	case EAppLogTypePICO::BodyTracking:
		Param = XRPortal;
		Value = BodyTracking;
		break;
	case EAppLogTypePICO::PICODeveloperWebsite:
		Param = XRPortal;
		Value = PICODeveloperWebsite;
		break;
	case EAppLogTypePICO::DownloadFromPICO_Official:
		Param = XRPortal;
		Value = DownloadFromPICO_Official;
		break;
	case EAppLogTypePICO::DownloadFromUnreal_Fab:
		Param = XRPortal;
		Value = DownloadFromUnreal_Fab;
		break;
	case EAppLogTypePICO::DownloadPICOForkOfUnrealEngine:
		Param = XRPortal;
		Value = DownloadPICOForkOfUnrealEngine;
		break;
	case EAppLogTypePICO::SpatialMeshActor:
		Param = XRToolKit;
		Value = SpatialMeshActor;
		break;
	case EAppLogTypePICO::SpatialAnchorActor:
		Param = XRToolKit;
		Value = SpatialAnchorActor;
		break;
	case EAppLogTypePICO::SceneCapturesGenerator:
		Param = XRToolKit;
		Value = SceneCapturesGenerator;
		break;
	case EAppLogTypePICO::PICOXRPawn:
		Param = XRToolKit;
		Value = PICOXRPawn;
		break;
	case EAppLogTypePICO::HandTrackingComponent:
		Param = XRToolKit;
		Value = HandTrackingComponent;
		break;
	case EAppLogTypePICO::EyeTrackingComponent:
		Param = XRToolKit;
		Value = EyeTrackingComponent;
		break;
	case EAppLogTypePICO::FaceTrackingComponent:
		Param = XRToolKit;
		Value = FaceTrackingComponent;
		break;
	case EAppLogTypePICO::DeferredShading:
		Param = XRSettings;
		Value = DeferredShading;
		break;
	case EAppLogTypePICO::ForwardShading:
		Param = XRSettings;
		Value = ForwardShading;
		break;
	case EAppLogTypePICO::MobileHDR:
		Param = XRSettings;
		Value = MobileHDR;
		break;
	case EAppLogTypePICO::LateLatching:
		Param = XRSettings;
		Value = LateLatching;
		break;
	case EAppLogTypePICO::ApplicationSpaceWarp:
		Param = XRSettings;
		Value = ApplicationSpaceWarp;
		break;
	case EAppLogTypePICO::ToneMapping:
		Param = XRSettings;
		Value = ToneMapping;
		break;
	case EAppLogTypePICO::MSAA:
		Param = XRSettings;
		Value = MSAA;
		break;
	case EAppLogTypePICO::FXAA:
		Param = XRSettings;
		Value = FXAA;
		break;
	case EAppLogTypePICO::TAA:
		Param = XRSettings;
		Value = TAA;
        break;
	case EAppLogTypePICO::MovableSpotlight:
		Param = XRSettings;
		Value = MovableSpotlight;
		break;
	case EAppLogTypePICO::None:
	default:
		break;
	}
	
    if (!Param.IsEmpty() && !Value.IsEmpty())
    {
    	PICO_AppLog::AddAppLog(Param, Value);
    	UE_LOG(LogAppLog, Verbose, TEXT("AddAppLog Param:%s Value:%s"), *Param, *Value);
    }
}
