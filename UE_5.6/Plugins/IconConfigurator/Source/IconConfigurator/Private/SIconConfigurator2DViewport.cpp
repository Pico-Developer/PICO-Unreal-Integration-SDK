// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "SIconConfigurator2DViewport.h"
#include "Engine/Texture2D.h"
#include "Rendering/DrawElements.h"

void SIconConfigurator2DViewport::Construct(const FArguments& InArgs)
{
	Layers = InArgs._LayerData;

	// Initialize 3 empty brushes
	for (int32 i = 0; i < 3; ++i)
	{
		LayerBrushes[i] = FSlateBrush();
		LayerBrushes[i].DrawAs = ESlateBrushDrawType::Image;
	}
	// Pure custom-draw widget; no child widgets
}

void SIconConfigurator2DViewport::Refresh()
{
	if (!Layers) return;

	for (int32 i = 0; i < Layers->Num() && i < 3; ++i)
	{
		const FIconLayerData& Layer = (*Layers)[i];

		UTexture2D* Tex = Layer.PreviewTexture.IsValid()
			? Layer.PreviewTexture.Get()
			: (Layer.ImportedTexture.IsValid() ? Layer.ImportedTexture.Get() : nullptr);

		if (Tex)
		{
			LayerBrushes[i].SetResourceObject(Tex);
			LayerBrushes[i].ImageSize = FVector2D(
				static_cast<float>(Tex->GetSizeX()),
				static_cast<float>(Tex->GetSizeY()));
		}
		else
		{
			LayerBrushes[i].SetResourceObject(nullptr);
			LayerBrushes[i].ImageSize = FVector2D::ZeroVector;
		}
	}

	Invalidate(EInvalidateWidgetReason::Paint);
}

FVector2D SIconConfigurator2DViewport::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(512.f, 512.f);
}

void SIconConfigurator2DViewport::DrawCheckerboard(
	const FGeometry& AllottedGeometry,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId) const
{
	const FVector2D ViewSize = AllottedGeometry.GetLocalSize();
	const float CellSize = 16.f;

	for (float y = 0.f; y < ViewSize.Y; y += CellSize)
	{
		for (float x = 0.f; x < ViewSize.X; x += CellSize)
		{
			const int32 CellX = FMath::FloorToInt(x / CellSize);
			const int32 CellY = FMath::FloorToInt(y / CellSize);
			const bool bDark = (CellX + CellY) % 2 == 0;
			const FLinearColor CellColor = bDark
				? FLinearColor(0.25f, 0.25f, 0.25f, 1.f)
				: FLinearColor(0.55f, 0.55f, 0.55f, 1.f);

			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(
					FVector2D(FMath::Min(CellSize, ViewSize.X - x),
					          FMath::Min(CellSize, ViewSize.Y - y)),
					FSlateLayoutTransform(FVector2D(x, y))),
				FCoreStyle::Get().GetBrush("WhiteBrush"),
				ESlateDrawEffect::None,
				CellColor);
		}
	}
}

int32 SIconConfigurator2DViewport::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	// 1. Checkerboard background
	DrawCheckerboard(AllottedGeometry, OutDrawElements, LayerId++);

	if (!Layers)
	{
		return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect,
			OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	}

	const FVector2D ViewSize = AllottedGeometry.GetLocalSize();

	// 2. Draw in order: index 0 (back) → 1 (mid) → 2 (front)
	for (int32 i = 0; i < Layers->Num() && i < 3; ++i)
	{
		const FIconLayerData& Layer = (*Layers)[i];
		if (!Layer.bVisible || Layer.Opacity <= 0.f)
		{
			continue;
		}

		const FSlateBrush& Brush = LayerBrushes[i];
		if (Brush.GetResourceObject() == nullptr)
		{
			continue;
		}

		const FVector2D TexSize = Brush.ImageSize;
		if (TexSize.X <= 0.f || TexSize.Y <= 0.f) continue;

		// Fit to viewport maintaining aspect ratio, then apply Layer.Scale
		const float FitScale = FMath::Min(ViewSize.X / TexSize.X, ViewSize.Y / TexSize.Y);
		const float DrawW = TexSize.X * FitScale * Layer.Scale;
		const float DrawH = TexSize.Y * FitScale * Layer.Scale;

		// Center + offset
		const float BaseX = (ViewSize.X - DrawW) * 0.5f + Layer.Offset.X;
		const float BaseY = (ViewSize.Y - DrawH) * 0.5f + Layer.Offset.Y;

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(
				FVector2D(DrawW, DrawH),
				FSlateLayoutTransform(FVector2D(BaseX, BaseY))),
			&Brush,
			ESlateDrawEffect::None,
			FLinearColor(1.f, 1.f, 1.f, Layer.Opacity));

		++LayerId;
	}

	return LayerId;
}
