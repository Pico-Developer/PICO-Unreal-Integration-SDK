// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_Verification.h"

#include "ISettingsModule.h"
#include "Settings/EditorSettings.h"
#include "UI/PXR_VerificationStyle.h"

#define LOCTEXT_NAMESPACE "ProjectVerification"
void FPICOXRProjectVerificationModule::StartupModule()
{
	FPICOXRVerificationStyle::Initialize();
	FPICOXRVerificationStyle::ReloadTextures();

	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		RegisterGeneralSettings(*SettingsModule);
		SettingsModule->RegisterViewer("PICO", *this);
	}

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(EditorSettingsTabName, FOnSpawnTab::CreateRaw(this, &FPICOXRProjectVerificationModule::HandleSpawnSettingsTab))
		.SetDisplayName(LOCTEXT("PICOSettingsTabTitle", "PICO Project Settings"))
		.SetMenuType(ETabSpawnerMenuType::Hidden)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "EditorPreferences.TabIcon"));

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FPICOXRProjectVerificationModule::RegisterMenus));
}


void FPICOXRProjectVerificationModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	GetMutableDefault<ULevelEditorPlaySettings>()->RegisterCommonResolutionsMenu();
}

	

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FPICOXRProjectVerificationModule, PICOXRProjectVerification)