// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "IconConfiguratorModule.h"
#include "SIconConfiguratorWidget.h"

#include "ToolMenus.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "FIconConfiguratorModule"

static const FName IconConfiguratorTabName("IconConfigurator");

void FIconConfiguratorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		IconConfiguratorTabName,
		FOnSpawnTab::CreateRaw(this, &FIconConfiguratorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("TabTitle", "PICO Icon Configurator"))
		.SetMenuType(ETabSpawnerMenuType::Enabled)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Layers"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory());
}

void FIconConfiguratorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(IconConfiguratorTabName);
}

TSharedRef<SDockTab> FIconConfiguratorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("TabTitle", "PICO Icon Configurator"))
		[
			SNew(SIconConfiguratorWidget)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FIconConfiguratorModule, IconConfigurator)
