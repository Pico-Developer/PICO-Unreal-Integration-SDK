// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_PortalModule.h"
#include "ISettingsModule.h"
#include "PICO_PerfPackDeviceSettingsCustomization.h"
#include "UI/PICO_VerificationStyle.h"
#include "PICO_AppLog.h"
#include "Interfaces/IPluginManager.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "PICOXRPortalModule"
void FPICOXRPortalModule::StartupModule()
{
	FPICOOpenXRVerificationStyle::Initialize();
	FPICOOpenXRVerificationStyle::ReloadTextures();

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
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FPICOXRPortalModule::RegisterMenus));

#if PLATFORM_WINDOWS
	FString AppLogDLLDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("PICOOpenXR"))->GetBaseDir(), TEXT("Source/PICOOpenXRPortal/AppLogSDK/Bin/"));
	AppLogrsHandle = FPlatformProcess::GetDllHandle(*(AppLogDLLDir + "applogrs.dll"));

	if (!AppLogrsHandle)
	{
		UE_LOG(LogAppLog, Error, TEXT("AppLogrs.dll not found"));
	}

	PICO_AppLog::InitAppLog();
#endif

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(
		"PICOPerfPackDeviceSettings",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPICOPerfPackDeviceSettingsCustomization::MakeInstance));
	PropertyEditorModule.NotifyCustomizationModuleChanged();
}

void FPICOXRPortalModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout("PICOPerfPackDeviceSettings");
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}

	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(EditorSettingsTabName);
	UnregisterSettings();

#if PLATFORM_WINDOWS
	if (AppLogrsHandle)
	{
		FPlatformProcess::FreeDllHandle(AppLogrsHandle);
		AppLogrsHandle = nullptr;
	}
#endif
	PICO_AppLog::ShutdownAppLog();
}


void FPICOXRPortalModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	GetMutableDefault<ULevelEditorPlaySettings>()->RegisterCommonResolutionsMenu();
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FPICOXRPortalModule, PICOOpenXRPortal)
