// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PXR_EditorStyle.h"


class FPICOXREditorCommands : public TCommands<FPICOXREditorCommands>
{
public:

	FPICOXREditorCommands()
		: TCommands<FPICOXREditorCommands>(TEXT("PICOXREditor"), NSLOCTEXT("Contexts", "PICOXREditor", "PICOXREditor Plugin"), NAME_None, TEXT("PICOXREditor.Common.Icon"))
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> PluginAction;
	TSharedPtr<FUICommandInfo> LaunchAPIReferenceURL;
	TSharedPtr<FUICommandInfo> LaunchSampleDemoURL;
	TSharedPtr<FUICommandInfo> LaunchOfficialWebsiteURL;
	TSharedPtr<FUICommandInfo> LaunchMarketplaceURL;
	TSharedPtr<FUICommandInfo> AddSampleInputs;
	TSharedPtr<FUICommandInfo> ClearSampleInputs;

};
