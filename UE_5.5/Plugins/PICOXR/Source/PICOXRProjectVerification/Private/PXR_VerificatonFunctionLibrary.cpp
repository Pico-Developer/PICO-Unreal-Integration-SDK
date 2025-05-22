// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_VerificatonFunctionLibrary.h"

#include "ISettingsModule.h"
#include "PXR_VerificationSubsystem.h"
#include "PXR_VerificationSettings.h"

bool UUPICOXRVerificationFunctionLibrary::bShowRestartEditorButton = false;

TArray<UVerificationObject*> UUPICOXRVerificationFunctionLibrary::GetFixedVerificationObjects()
{
	UPXR_VerificationSubsystem* ProcessorSubsystem = GEngine->GetEngineSubsystem<UPXR_VerificationSubsystem>();
	return ProcessorSubsystem->FixedVerificationObjects;
}

TArray<UVerificationObject*> UUPICOXRVerificationFunctionLibrary::GetRequiredVerificationObjects()
{
	UPXR_VerificationSubsystem* ProcessorSubsystem = GEngine->GetEngineSubsystem<UPXR_VerificationSubsystem>();
	return ProcessorSubsystem->RequiredVerificationObjects;
}

static UPICOXRVerificatonEventManager* EventManagerInstance = nullptr;


UPICOXRVerificatonEventManager* UUPICOXRVerificationFunctionLibrary::PXR_GetVerificationEventManager()
{
	return UPICOXRVerificatonEventManager::GetInstance();
}

bool UUPICOXRVerificationFunctionLibrary::HasVerificationNeedRestart()
{
	return bShowRestartEditorButton;
}

void UUPICOXRVerificationFunctionLibrary::SetVerificationNeedRestart(bool bRestart)
{
	bShowRestartEditorButton=bRestart;
}

void UUPICOXRVerificationFunctionLibrary::UpdateVerificationStatus()
{
	UPXR_VerificationSubsystem* ProcessorSubsystem = GEngine->GetEngineSubsystem<UPXR_VerificationSubsystem>();
	bool bRestartEditorButton=false;
	if(ProcessorSubsystem->Refresh(bRestartEditorButton))
	{
		UPICOXRVerificatonEventManager::GetInstance()->VerificatonUpdatedDelegate.Broadcast();

		if(bRestartEditorButton)
		{
			SetVerificationNeedRestart(bRestartEditorButton);
		}
	}
}

void UUPICOXRVerificationFunctionLibrary::FixAllVerifications()
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

bool UUPICOXRVerificationFunctionLibrary::IsShowPortalOnEditorStart()
{
	UPXR_VerificationSettings* VerificationSettings=GetMutableDefault<UPXR_VerificationSettings>();
	return VerificationSettings->bShowPortalOnEditorStart;
}

void UUPICOXRVerificationFunctionLibrary::SetShowPortalOnEditorStart(bool bShowPortalOnEditorStart)
{
	UPXR_VerificationSettings* VerificationSettings=GetMutableDefault<UPXR_VerificationSettings>();
	VerificationSettings->bShowPortalOnEditorStart=bShowPortalOnEditorStart;

	VerificationSettings->SaveConfig();
}

void UUPICOXRVerificationFunctionLibrary::LaunchPICOSettings()
{
	FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer(FName("PICO"), FName("General"), FName("PICOXR Settings"));
}

FString UUPICOXRVerificationFunctionLibrary::GetVersionString()
{
	return FString("Version 3.2.0");
}

bool UUPICOXRVerificationFunctionLibrary::IsXRPluginVerified()
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

void UUPICOXRVerificationFunctionLibrary::FixXRPluginConfig()
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

bool UUPICOXRVerificationFunctionLibrary::IsVulkanVerified()
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

void UUPICOXRVerificationFunctionLibrary::FixVulkanConfig()
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

bool UUPICOXRVerificationFunctionLibrary::IsMultiviewVerified()
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

void UUPICOXRVerificationFunctionLibrary::FixMultiviewConfig()
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

bool UUPICOXRVerificationFunctionLibrary::IsAndroidSDKVersionVerified()
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

void UUPICOXRVerificationFunctionLibrary::FixAndroidSDKVersionConfig()
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

