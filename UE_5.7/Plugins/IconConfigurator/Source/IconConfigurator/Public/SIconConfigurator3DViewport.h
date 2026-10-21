// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "PreviewScene.h"
#include "IconLayerData.h"

class FIconConfigurator3DViewportClient;

/**
 * 3D preview viewport Slate widget, wrapping FIconConfigurator3DViewportClient
 */
class SIconConfigurator3DViewport : public SEditorViewport
{
public:
	SLATE_BEGIN_ARGS(SIconConfigurator3DViewport) {}
		SLATE_ARGUMENT(TArray<FIconLayerData>*, LayerData)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SIconConfigurator3DViewport() override;

	/** Refresh 3D planes after textures are updated */
	void Refresh();

	/** Switch the layer data pointer (used when tab changes) */
	void SetLayerData(TArray<FIconLayerData>* InLayers) { Layers = InLayers; }

	/** Reset camera */
	void ResetCamera();

protected:
	// SEditorViewport interface
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;

private:
	TSharedPtr<FIconConfigurator3DViewportClient> ViewportClient;
	TSharedPtr<FPreviewScene> PreviewScene;
	TArray<FIconLayerData>* Layers = nullptr;
};
