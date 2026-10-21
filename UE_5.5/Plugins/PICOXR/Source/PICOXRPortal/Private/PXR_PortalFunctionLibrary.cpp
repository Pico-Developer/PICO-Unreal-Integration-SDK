// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_PortalFunctionLibrary.h"

#include "ISettingsModule.h"
#include "PXR_AppLog.h"
#include "PXR_PortalModule.h"
#include "PXR_PortalSubsystem.h"
#include "PXR_PortalSettings.h"
#include "Interfaces/IPluginManager.h"
#include "Engine/Engine.h"

bool UPICOXRPortalFunctionLibrary::bShowRestartEditorButton = false;
FString UPICOXRPortalFunctionLibrary::XRPortal = "PICOXRPortal";
FString UPICOXRPortalFunctionLibrary::XRToolKit = "PICOXRToolKit";
FString UPICOXRPortalFunctionLibrary::XRSettings = "PICOXRSettings";
FString UPICOXRPortalFunctionLibrary::XREngineVersion = "EngineVersion";
FString UPICOXRPortalFunctionLibrary::XRSDKVersion = "SDKVersion";
FString UPICOXRPortalFunctionLibrary::SDKType = "UnrealSDKType";
FString UPICOXRPortalFunctionLibrary::EngineType = "UnrealEngineType";
FString UPICOXRPortalFunctionLibrary::PICOXRSDK = "PICOXRSDK";
FString UPICOXRPortalFunctionLibrary::OpenXRSDK = "OpenXRSDK";
FString UPICOXRPortalFunctionLibrary::OfficialEngine = "OfficialEngine";
FString UPICOXRPortalFunctionLibrary::PICOCustomEngine = "PICOCustomEngine";

FString UPICOXRPortalFunctionLibrary::XRPortalStartUp = "PICOXRPortalStartUp";
FString UPICOXRPortalFunctionLibrary::ProjectVerificationStartUp = "ProjectVerificationStartUp";
FString UPICOXRPortalFunctionLibrary::XRPICOSettingsStartUp = "PICOXRPICOSettingsStartUp";
FString UPICOXRPortalFunctionLibrary::PICODeveloperCenter = "PICODeveloperCenter";
FString UPICOXRPortalFunctionLibrary::PICOEmulator = "PICOEmulator";
FString UPICOXRPortalFunctionLibrary::GetStartedSample = "GetStartedSample";
FString UPICOXRPortalFunctionLibrary::PlatformSample = "PlatformSample";
FString UPICOXRPortalFunctionLibrary::SpatialAudioSample = "SpatialAudioSample";
FString UPICOXRPortalFunctionLibrary::MRSample = "MRSample";
FString UPICOXRPortalFunctionLibrary::HandTrackingSample = "HandTrackingSample";
FString UPICOXRPortalFunctionLibrary::ControllerInteraction = "ControllerInteraction";
FString UPICOXRPortalFunctionLibrary::BodyTracking = "BodyTracking";
FString UPICOXRPortalFunctionLibrary::PICODeveloperWebsite = "PICODeveloperWebsite";
FString UPICOXRPortalFunctionLibrary::DownloadFromPICO_Official = "DownloadFromPICO_Official";
FString UPICOXRPortalFunctionLibrary::DownloadFromUnreal_Fab = "DownloadFromUnreal_Fab";
FString UPICOXRPortalFunctionLibrary::DownloadPICOForkOfUnrealEngine = "DownloadPICOForkOfUnrealEngine";

FString UPICOXRPortalFunctionLibrary::SpatialMeshActor = "SpatialMeshActor";
FString UPICOXRPortalFunctionLibrary::SpatialAnchorActor = "SpatialAnchorActor";
FString UPICOXRPortalFunctionLibrary::SceneCapturesGenerator = "SceneCapturesGenerator";
FString UPICOXRPortalFunctionLibrary::PICOXRPawn = "PICOXRPawn";
FString UPICOXRPortalFunctionLibrary::HandTrackingComponent = "HandTrackingComponent";
FString UPICOXRPortalFunctionLibrary::EyeTrackingComponent = "EyeTrackingComponent";
FString UPICOXRPortalFunctionLibrary::FaceTrackingComponent = "FaceTrackingComponent";

FString UPICOXRPortalFunctionLibrary::MobileHDR = "MobileHDR";
FString UPICOXRPortalFunctionLibrary::DeferredShading = "DeferredShading";
FString UPICOXRPortalFunctionLibrary::ForwardShading = "ForwardShading";
FString UPICOXRPortalFunctionLibrary::LateLatching = "LateLatching";
FString UPICOXRPortalFunctionLibrary::MovableSpotlight = "MovableSpotlight";
FString UPICOXRPortalFunctionLibrary::ApplicationSpaceWarp = "ApplicationSpaceWarp";
FString UPICOXRPortalFunctionLibrary::ToneMapping = "ToneMapping";
FString UPICOXRPortalFunctionLibrary::FXAA = "FXAA";
FString UPICOXRPortalFunctionLibrary::TAA = "TAA";
FString UPICOXRPortalFunctionLibrary::MSAA = "MSAA";


TArray<UVerificationObject*> UPICOXRPortalFunctionLibrary::GetFixedVerificationObjects()
{
	UPXR_PortalSubsystem* ProcessorSubsystem = GEngine->GetEngineSubsystem<UPXR_PortalSubsystem>();
	return ProcessorSubsystem->FixedVerificationObjects;
}

TArray<UVerificationObject*> UPICOXRPortalFunctionLibrary::GetRequiredVerificationObjects()
{
	UPXR_PortalSubsystem* ProcessorSubsystem = GEngine->GetEngineSubsystem<UPXR_PortalSubsystem>();
	return ProcessorSubsystem->RequiredVerificationObjects;
}

static UPICOXRVerificatonEventManager* EventManagerInstance = nullptr;


UPICOXRVerificatonEventManager* UPICOXRPortalFunctionLibrary::PXR_GetVerificationEventManager()
{
	return UPICOXRVerificatonEventManager::GetInstance();
}

bool UPICOXRPortalFunctionLibrary::HasVerificationNeedRestart()
{
	return bShowRestartEditorButton;
}

void UPICOXRPortalFunctionLibrary::SetVerificationNeedRestart(bool bRestart)
{
	bShowRestartEditorButton=bRestart;
}

void UPICOXRPortalFunctionLibrary::StartUpdatePortalTick()
{
	UPXR_PortalSubsystem* PortalSubsystem = GEngine->GetEngineSubsystem<UPXR_PortalSubsystem>();
	bool bRestartEditorButton=false;
	if(PortalSubsystem&&PortalSubsystem->Refresh(bRestartEditorButton))
	{
		UPICOXRVerificatonEventManager::GetInstance()->VerificatonUpdatedDelegate.Broadcast();

		if(bRestartEditorButton)
		{
			SetVerificationNeedRestart(bRestartEditorButton);
		}
	}
}

void UPICOXRPortalFunctionLibrary::FixAllVerifications()
{
	for (auto VerificationObject : GetRequiredVerificationObjects())
	{
		VerificationObject->PerformVerificationFix();
	}
}

UPICOXRVerificatonEventManager* UPICOXRVerificatonEventManager::GetInstance()
{
	if (EventManagerInstance == nullptr)
	{
		EventManagerInstance = NewObject<UPICOXRVerificatonEventManager>();
		EventManagerInstance->AddToRoot();
	}
	return EventManagerInstance;
}

bool UPICOXRPortalFunctionLibrary::IsShowPortalOnEditorStart()
{
	UPXR_PortalSettings* VerificationSettings=GetMutableDefault<UPXR_PortalSettings>();
	return VerificationSettings->bShowPortalOnEditorStart;
}

bool UPICOXRPortalFunctionLibrary::IsBehaviorDataCollectionEnabled()
{
	UPXR_PortalSettings* VerificationSettings=GetMutableDefault<UPXR_PortalSettings>();
	return VerificationSettings->bBehaviorDataCollectionEnabled;
}

void UPICOXRPortalFunctionLibrary::SetShowPortalOnEditorStart(bool bShowPortalOnEditorStart)
{
	UPXR_PortalSettings* VerificationSettings=GetMutableDefault<UPXR_PortalSettings>();
	VerificationSettings->bShowPortalOnEditorStart=bShowPortalOnEditorStart;

	VerificationSettings->SaveConfig();
}

void UPICOXRPortalFunctionLibrary::SetBehaviorDataCollectionEnabled(bool bBehaviorDataCollectionEnabled)
{
	UPXR_PortalSettings* VerificationSettings=GetMutableDefault<UPXR_PortalSettings>();
	VerificationSettings->bBehaviorDataCollectionEnabled=bBehaviorDataCollectionEnabled;

	VerificationSettings->SaveConfig();
}

void UPICOXRPortalFunctionLibrary::LaunchPICOSettings()
{
	FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer(FName("PICO"), FName("General"), FName("PICOXR Settings"));
}

FString UPICOXRPortalFunctionLibrary::GetPICOSDKVersionString()
{
	if (TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("PICOXR")))
	{
		if (Plugin->IsEnabled())
		{
			return FString("Version ")+Plugin->GetDescriptor().VersionName;
		}
	}
	
	return FString("Version Unknown");
}

bool UPICOXRPortalFunctionLibrary::IsXRPluginVerified()
{
	for (auto VerificationObject : GetRequiredVerificationObjects())
	{
		if (VerificationObject->GetDisplayName().ToString()==FString("Disable OpenXR Plugin"))
		{
			return false;
		}
	}
	
	return true;
}

void UPICOXRPortalFunctionLibrary::FixXRPluginConfig()
{
	for (auto VerificationObject : GetRequiredVerificationObjects())
	{
		if (VerificationObject->GetDisplayName().ToString()==FString("Disable OpenXR Plugin"))
		{
			VerificationObject->PerformVerificationFix();
			return;
		}
	}
}

bool UPICOXRPortalFunctionLibrary::IsVulkanVerified()
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

void UPICOXRPortalFunctionLibrary::FixVulkanConfig()
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

bool UPICOXRPortalFunctionLibrary::IsMultiviewVerified()
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

void UPICOXRPortalFunctionLibrary::FixMultiviewConfig()
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

bool UPICOXRPortalFunctionLibrary::IsAndroidSDKVersionVerified()
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

void UPICOXRPortalFunctionLibrary::FixAndroidSDKVersionConfig()
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

void UPICOXRPortalFunctionLibrary::AddAppLogByType(EPICOXRAppLogType AppLogType,const FString& AppLogString)
{
	if (!IsBehaviorDataCollectionEnabled())
	{
		return;
	}
	
	FString Param;
	FString Value;
	switch (AppLogType)
	{
	case EPICOXRAppLogType::EngineVersion:
		Param = XREngineVersion;
		Value = AppLogString;
		break;
	case EPICOXRAppLogType::SDKVersion:
		Param = XRSDKVersion;
		Value = AppLogString;
		break;
	case EPICOXRAppLogType::SDKType:
		Param = SDKType;
		Value = PICOXRSDK;
		break;
	case EPICOXRAppLogType::EngineType:
    	Param = EngineType;
#ifdef PICO_CUSTOM_ENGINE
		Value = PICOCustomEngine;
#else
		Value = OfficialEngine;
#endif
    	break;
	case EPICOXRAppLogType::PICOXRPortalStartUp:
		Param = XRPortal;
		Value = XRPortalStartUp;
		break;
	case EPICOXRAppLogType::PICOSettingsStartUp:
		Param = XRPortal;
		Value = XRPICOSettingsStartUp;
		break;
	case EPICOXRAppLogType::ProjectVerificationStartUp:
		Param = XRPortal;
		Value = ProjectVerificationStartUp;
		break;
	case EPICOXRAppLogType::PICODeveloperCenter:
		Param = XRPortal;
		Value = PICODeveloperCenter;
		break;
	case EPICOXRAppLogType::PICOEmulator:
		Param = XRPortal;
		Value = PICOEmulator;
		break;
	case EPICOXRAppLogType::GetStartedSample:
		Param = XRPortal;
		Value = GetStartedSample;
		break;
	case EPICOXRAppLogType::PlatformSample:
		Param = XRPortal;
		Value = PlatformSample;
		break;
	case EPICOXRAppLogType::SpatialAudioSample:
		Param = XRPortal;
		Value = SpatialAudioSample;
		break;
	case EPICOXRAppLogType::MRSample:
		Param = XRPortal;
		Value = MRSample;
		break;
	case EPICOXRAppLogType::HandTrackingSample:
		Param = XRPortal;
		Value = HandTrackingSample;
		break;
	case EPICOXRAppLogType::ControllerInteraction:
		Param = XRPortal;
		Value = ControllerInteraction;
		break;
	case EPICOXRAppLogType::BodyTracking:
		Param = XRPortal;
		Value = BodyTracking;
		break;
	case EPICOXRAppLogType::PICODeveloperWebsite:
		Param = XRPortal;
		Value = PICODeveloperWebsite;
		break;
	case EPICOXRAppLogType::DownloadFromPICO_Official:
		Param = XRPortal;
		Value = DownloadFromPICO_Official;
		break;
	case EPICOXRAppLogType::DownloadFromUnreal_Fab:
		Param = XRPortal;
		Value = DownloadFromUnreal_Fab;
		break;
	case EPICOXRAppLogType::DownloadPICOForkOfUnrealEngine:
		Param = XRPortal;
		Value = DownloadPICOForkOfUnrealEngine;
		break;
	case EPICOXRAppLogType::SpatialMeshActor:
		Param = XRToolKit;
		Value = SpatialMeshActor;
		break;
	case EPICOXRAppLogType::SpatialAnchorActor:
		Param = XRToolKit;
		Value = SpatialAnchorActor;
		break;
	case EPICOXRAppLogType::SceneCapturesGenerator:
		Param = XRToolKit;
		Value = SceneCapturesGenerator;
		break;
	case EPICOXRAppLogType::PICOXRPawn:
		Param = XRToolKit;
		Value = PICOXRPawn;
		break;
	case EPICOXRAppLogType::HandTrackingComponent:
		Param = XRToolKit;
		Value = HandTrackingComponent;
		break;
	case EPICOXRAppLogType::EyeTrackingComponent:
		Param = XRToolKit;
		Value = EyeTrackingComponent;
		break;
	case EPICOXRAppLogType::FaceTrackingComponent:
		Param = XRToolKit;
		Value = FaceTrackingComponent;
		break;
	case EPICOXRAppLogType::DeferredShading:
		Param = XRSettings;
		Value = DeferredShading;
		break;
	case EPICOXRAppLogType::ForwardShading:
		Param = XRSettings;
		Value = ForwardShading;
		break;
	case EPICOXRAppLogType::MobileHDR:
		Param = XRSettings;
		Value = MobileHDR;
		break;
	case EPICOXRAppLogType::LateLatching:
		Param = XRSettings;
		Value = LateLatching;
		break;
	case EPICOXRAppLogType::ApplicationSpaceWarp:
		Param = XRSettings;
		Value = ApplicationSpaceWarp;
		break;
	case EPICOXRAppLogType::ToneMapping:
		Param = XRSettings;
		Value = ToneMapping;
		break;
	case EPICOXRAppLogType::MSAA:
		Param = XRSettings;
		Value = MSAA;
		break;
	case EPICOXRAppLogType::FXAA:
		Param = XRSettings;
		Value = FXAA;
		break;
	case EPICOXRAppLogType::TAA:
		Param = XRSettings;
		Value = TAA;
        break;
	case EPICOXRAppLogType::MovableSpotlight:
		Param = XRSettings;
		Value = MovableSpotlight;
		break;
	case EPICOXRAppLogType::None:
	default:
		break;
	}
	
    if (!Param.IsEmpty() && !Value.IsEmpty())
    {
    	PXR_AppLog::AddAppLog(Param, Value);
    	UE_LOG(LogAppLog, Verbose, TEXT("AddAppLog Param:%s Value:%s"), *Param, *Value);
    }
}
