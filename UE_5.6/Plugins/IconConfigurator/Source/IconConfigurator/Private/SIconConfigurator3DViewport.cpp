// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "SIconConfigurator3DViewport.h"
#include "IconConfigurator3DViewportClient.h"

#include "PreviewScene.h"
#include "Slate/SceneViewport.h"
#include "Widgets/Layout/SBox.h"

void SIconConfigurator3DViewport::Construct(const FArguments& InArgs)
{
	Layers = InArgs._LayerData;

	PreviewScene = MakeShared<FPreviewScene>(FPreviewScene::ConstructionValues()
		.SetEditor(true));

	SEditorViewport::Construct(SEditorViewport::FArguments());
}

SIconConfigurator3DViewport::~SIconConfigurator3DViewport()
{
	if (ViewportClient.IsValid())
	{
		ViewportClient->Viewport = nullptr;
	}
}

TSharedRef<FEditorViewportClient> SIconConfigurator3DViewport::MakeEditorViewportClient()
{
	ViewportClient = MakeShared<FIconConfigurator3DViewportClient>(PreviewScene.Get());
	return ViewportClient.ToSharedRef();
}

void SIconConfigurator3DViewport::Refresh()
{
	if (ViewportClient.IsValid() && Layers)
	{
		ViewportClient->UpdateLayerTextures(*Layers);
	}
}

void SIconConfigurator3DViewport::ResetCamera()
{
	if (ViewportClient.IsValid())
	{
		ViewportClient->ResetCamera();
	}
}
