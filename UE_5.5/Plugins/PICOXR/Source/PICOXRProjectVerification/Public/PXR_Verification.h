// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "ISettingsCategory.h"
#include "ISettingsContainer.h"
#include "ISettingsEditorModel.h"
#include "ISettingsEditorModule.h"
#include "ISettingsModule.h"
#include "ISettingsViewer.h"
#include "PXR_HMDRuntimeSettings.h"
#include "PXR_VerificationSettings.h"
#include "Modules/ModuleManager.h"
#include "Settings/EditorSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogProjectVerificationTool, Log, All);
static const FName EditorSettingsTabName("PICO Project Settings");

#define LOCTEXT_NAMESPACE "ProjectVerification"

class FPICOXRProjectVerificationModule : public IModuleInterface, public ISettingsViewer
{
public:
    virtual void StartupModule() override;

    void RegisterMenus();

	virtual void ShowSettings( const FName& CategoryName, const FName& SectionName ) override
	{
		FGlobalTabmanager::Get()->TryInvokeTab(EditorSettingsTabName);
		ISettingsEditorModelPtr SettingsEditorModel = SettingsEditorModelPtr.Pin();

		if (SettingsEditorModel.IsValid())
		{
			ISettingsCategoryPtr Category = SettingsEditorModel->GetSettingsContainer()->GetCategory(CategoryName);

			if (Category.IsValid())
			{
				SettingsEditorModel->SelectSection(Category->GetSection(SectionName));
			}
		}
	}

	virtual void ShutdownModule() override
	{
		UToolMenus::UnRegisterStartupCallback(this);
		UToolMenus::UnregisterOwner(this);

		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(EditorSettingsTabName);
		UnregisterSettings();
	}

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}
    /**
	 * Registers general Editor settings.
	 *
	 * @param SettingsModule A reference to the settings module.
	 */
	void RegisterGeneralSettings( ISettingsModule& SettingsModule )
	{

		// loading & saving features
		SettingsModule.RegisterSettings("PICO", "General", "PICOXRSettings",
			LOCTEXT("PICOXRSettingsName", "PICOXR Settings"),
			LOCTEXT("PICOXRSettingsDescription", "Change how the PICOXR Settings."),
			GetMutableDefault<UPICOXRSettings>()
		);

		// loading & saving features
		SettingsModule.RegisterSettings("PICO", "General", "PICORenderingSettings",
			LOCTEXT("PICO Rendering SettingsName", "PICO Rendering Settings"),
			LOCTEXT("PICO Rendering SettingsDescription", "Change how the PICO Rendering Settings."),
			GetMutableDefault<UPXR_RenderSettings>()
		);
		

	}


	/** Unregisters all settings. */
	void UnregisterSettings()
	{
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			SettingsModule->UnregisterViewer("PICO");

			// general settings
			SettingsModule->UnregisterSettings("PICO", "General", "InputBindings");
			SettingsModule->UnregisterSettings("PICO", "General", "LoadingSaving");
			SettingsModule->UnregisterSettings("PICO", "General", "GameAgnostic");
			SettingsModule->UnregisterSettings("PICO", "General", "UserSettings");
			SettingsModule->UnregisterSettings("PICO", "General", "AutomationTest");
			SettingsModule->UnregisterSettings("PICO", "General", "Internationalization");
			SettingsModule->UnregisterSettings("PICO", "General", "Experimental");
			SettingsModule->UnregisterSettings("PICO", "General", "CrashReporter");			

			// level editor settings
			SettingsModule->UnregisterSettings("PICO", "LevelEditor", "PlayIn");
			SettingsModule->UnregisterSettings("PICO", "LevelEditor", "Viewport");

			// other tools
			SettingsModule->UnregisterSettings("PICO", "ContentEditors", "ContentBrowser");
			SettingsModule->UnregisterSettings("PICO", "ContentEditors", "GraphEditor");
			SettingsModule->UnregisterSettings("PICO", "ContentEditors", "Persona");
			SettingsModule->UnregisterSettings("PICO", "ContentEditors", "CurveEditor");
		}
	}

private:

	/** Handles creating the editor settings tab. */
	TSharedRef<SDockTab> HandleSpawnSettingsTab( const FSpawnTabArgs& SpawnTabArgs )
	{
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
		TSharedRef<SWidget> SettingsEditor = SNullWidget::NullWidget;

		if (SettingsModule != nullptr)
		{
			ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("PICO");

			if (SettingsContainer.IsValid())
			{
				ISettingsEditorModule& SettingsEditorModule = FModuleManager::GetModuleChecked<ISettingsEditorModule>("SettingsEditor");
				ISettingsEditorModelRef SettingsEditorModel = SettingsEditorModule.CreateModel(SettingsContainer.ToSharedRef());

				SettingsEditor = SettingsEditorModule.CreateEditor(SettingsEditorModel);
				SettingsEditorModelPtr = SettingsEditorModel;
			}
		}

		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				SettingsEditor
			];
	}

private:

	/** Holds a pointer to the settings editor's view model. */
	TWeakPtr<ISettingsEditorModel> SettingsEditorModelPtr;
};

#undef LOCTEXT_NAMESPACE