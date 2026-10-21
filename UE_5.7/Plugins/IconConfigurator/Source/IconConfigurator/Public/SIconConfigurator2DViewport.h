// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Styling/SlateBrush.h"
#include "IconLayerData.h"

/**
 * 2D preview viewport: composites 3 images in back/mid/front order.
 * Supports checkerboard background; each layer has independent Opacity / Offset / Scale / Visible.
 */
class SIconConfigurator2DViewport : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SIconConfigurator2DViewport) {}
		SLATE_ARGUMENT(TArray<FIconLayerData>*, LayerData)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Notify that layer data has changed; rebuild brushes and repaint */
	void Refresh();

	/** Switch the layer data pointer (used when tab changes) */
	void SetLayerData(TArray<FIconLayerData>* InLayers) { Layers = InLayers; }

	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

private:
	TArray<FIconLayerData>* Layers = nullptr;

	/**
	 * FSlateBrush per layer (value type; lifetime bound to the widget, safe to pass to render thread).
	 * Index 0=back, 1=mid, 2=front.
	 */
	FSlateBrush LayerBrushes[3];

	/** Draw checkerboard background */
	void DrawCheckerboard(
		const FGeometry& AllottedGeometry,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId) const;
};
