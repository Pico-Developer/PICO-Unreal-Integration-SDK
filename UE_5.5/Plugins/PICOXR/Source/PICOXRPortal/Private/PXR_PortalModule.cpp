// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_PortalModule.h"
#include "ISettingsModule.h"
#include "UI/PXR_VerificationStyle.h"
#include "PXR_AppLog.h"
#include "Interfaces/IPluginManager.h"
#include "Settings/LevelEditorPlaySettings.h"

#define LOCTEXT_NAMESPACE "PICOXRPortalModule"
void FPICOXRPortalModule::StartupModule()
{
	FPICOXRVerificationStyle::Initialize();
	FPICOXRVerificationStyle::ReloadTextures();

	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		RegisterGeneralSettings(*SettingsModule);
		SettingsModule->RegisterViewer("PICO", *this);
	}

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(EditorSettingsTabName, FOnSpawnTab::CreateRaw(this, &FPICOXRPortalModule::HandleSpawnSettingsTab))
		.SetDisplayName(LOCTEXT("PICOSettingsTabTitle", "PICO Project Settings"))
		.SetMenuType(ETabSpawnerMenuType::Hidden)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "EditorPreferences.TabIcon"));
	FString AppLogDLLDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("PICOXR"))->GetBaseDir(), TEXT("Source/ThirdParty/AppLog/Bin/"));
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FPICOXRPortalModule::RegisterMenus));
	AppLogrsHandle = FPlatformProcess::GetDllHandle(*(AppLogDLLDir + "applogrs.dll"));

	if (!AppLogrsHandle)
	{
		UE_LOG(LogAppLog, Error, TEXT("AppLogrs.dll not found"));
	}

	PXR_AppLog::InitAppLog();
}


void FPICOXRPortalModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	GetMutableDefault<ULevelEditorPlaySettings>()->RegisterCommonResolutionsMenu();
}

	

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FPICOXRPortalModule, PICOXRPortal)