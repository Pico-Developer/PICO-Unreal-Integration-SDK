// Copyright® PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "GameProjectGenerationModule.h"
#include "PXR_Verification.h"
#include "Interfaces/IPluginManager.h"
#include "Interfaces/IProjectManager.h"


#define PICOXR_SYNC_SETTINGS(SettingsClass1, SettingsClass2,PropertyName) \
do {                                                                   \
SettingsClass1* Settings1 = GetMutableDefault<SettingsClass1>();     \
SettingsClass2* Settings2 = GetMutableDefault<SettingsClass2>();     \
if (Settings1 && Settings2) {                                                   \
Settings1->PropertyName = Settings2->PropertyName;                       \
Settings1->UpdateSinglePropertyInConfigFile(                   \
Settings1->GetClass()->FindPropertyByName(                 \
GET_MEMBER_NAME_CHECKED(SettingsClass1, PropertyName)),\
Settings1->GetDefaultConfigFilename());                    \
}                                                                 \
} while (0)


#define PICOXR_UPDATE_SETTINGS(SettingsClass, PropertyName, PropertyValue) \
do {                                                                   \
	SettingsClass* Settings = GetMutableDefault<SettingsClass>();     \
	if (Settings) {                                                   \
	Settings->PropertyName = PropertyValue;                       \
	Settings->UpdateSinglePropertyInConfigFile(                   \
	Settings->GetClass()->FindPropertyByName(                 \
	GET_MEMBER_NAME_CHECKED(SettingsClass, PropertyName)),\
	Settings->GetDefaultConfigFilename());                    \
	}                                                                 \
} while (0)

class FVerificationUtils
{
public:



	static bool IsPluginEnabled(const FString& PluginName)
	{
		const auto Plugin = IPluginManager::Get().FindPlugin(PluginName);
		if (!Plugin)
		{
			return false;
		}

		return Plugin->IsEnabled();
	}

	static void FindPluginDependencies(const FString& Name, TSet<FString>& Dependencies, TMap<FString, IPlugin*>& NameToPlugin)
	{
		IPlugin* Plugin = NameToPlugin.FindRef(Name);
		if (Plugin != nullptr)
		{
			for (const FPluginReferenceDescriptor& Reference : Plugin->GetDescriptor().Plugins)
			{
				if (Reference.bEnabled && !Dependencies.Contains(Reference.Name))
				{
					Dependencies.Add(Reference.Name);
					FindPluginDependencies(Reference.Name, Dependencies, NameToPlugin);
				}
			}
		}
	}

	static bool DisablePlugin(const FString& PluginName)
	{
		// Get all the plugins we know about
		TArray<TSharedRef<IPlugin>> EnabledPlugins = IPluginManager::Get().GetEnabledPlugins();

		// Build a map of plugin by name
		TMap<FString, IPlugin*> NameToPlugin;
		for (TSharedRef<IPlugin>& EnabledPlugin : EnabledPlugins)
		{
			NameToPlugin.FindOrAdd(EnabledPlugin->GetName()) = &(EnabledPlugin.Get());
		}
		// Find all the plugins which are dependent on this plugin
		TArray<FString> DependentPluginNames;
		for (TSharedRef<IPlugin>& EnabledPlugin : EnabledPlugins)
		{
			FString EnabledPluginName = EnabledPlugin->GetName();

			TSet<FString> Dependencies;
			FindPluginDependencies(EnabledPluginName, Dependencies, NameToPlugin);

			if (Dependencies.Num() > 0 && Dependencies.Contains(PluginName))
			{
				FText Caption = FText::FromString("Disable Dependencies");
				FText Message = FText::Format(FText::FromString("This plugin is required by {0}. Would you like to disable it as well?"), FText::FromString(EnabledPluginName));
				if (FMessageDialog::Open(EAppMsgType::YesNo, Message, Caption) == EAppReturnType::No)
				{
					return false;
				}
				DependentPluginNames.Add(EnabledPluginName);
			}
		}

		// Disable all the dependent plugins too
		for (const FString& DependentPluginName : DependentPluginNames)
		{
			FText FailureMessage;
			if (!IProjectManager::Get().SetPluginEnabled(DependentPluginName, false, FailureMessage))
			{
				FMessageDialog::Open(EAppMsgType::Ok, FailureMessage);
			}
		}

		// Finally, enable/disable the plugin we selected
		FText FailMessage;
		bool bSuccess = IProjectManager::Get().SetPluginEnabled(PluginName, false, FailMessage);

		if (bSuccess && IProjectManager::Get().IsCurrentProjectDirty())
		{
			FGameProjectGenerationModule::Get().TryMakeProjectFileWriteable(FPaths::GetProjectFilePath());
			bSuccess = IProjectManager::Get().SaveCurrentProjectToDisk(FailMessage);
		}


		return bSuccess;
	}
};
