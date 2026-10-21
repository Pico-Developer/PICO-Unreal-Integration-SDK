// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICOHapticAssetThumbnailRenderer.h"
#include "PICOHapticAsset.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"

void UPICOHapticAssetThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	UPICOHapticAsset* HapticAsset = Cast<UPICOHapticAsset>(Object);
	if (!HapticAsset)
	{
		return;
	}

	// Draw background
	FCanvasTileItem BackgroundTile(FVector2D(X, Y), FVector2D(Width, Height), FLinearColor(0.1f, 0.1f, 0.15f));
	BackgroundTile.BlendMode = SE_BLEND_Opaque;
	Canvas->DrawItem(BackgroundTile);

	const float MarginX = Width * 0.1f;
	const float MarginY = Height * 0.1f;
	const float DrawWidth = Width - 2 * MarginX;
	const float DrawHeight = Height - 2 * MarginY;

	auto DrawWaveform = [&](const TArray<FPICOHapticFrameData>& PatternData, float PosX, float PosY, float WaveWidth, float WaveHeight, const FLinearColor& Color)
	{
		if (PatternData.Num() < 2)
		{
			return;
		}

		const float StepX = WaveWidth / FMath::Max(1, PatternData.Num() - 1);

		for (int32 i = 0; i < PatternData.Num() - 1; ++i)
		{
			const float X1 = PosX + i * StepX;
			const float X2 = PosX + (i + 1) * StepX;
			const float NormalizedGain1 = FMath::Clamp(PatternData[i].Gain, 0.0f, 1.0f);
			const float NormalizedGain2 = FMath::Clamp(PatternData[i + 1].Gain, 0.0f, 1.0f);
			const float Y1 = PosY + WaveHeight - (NormalizedGain1 * WaveHeight);
			const float Y2 = PosY + WaveHeight - (NormalizedGain2 * WaveHeight);

			FCanvasLineItem LineItem(FVector2D(X1, Y1), FVector2D(X2, Y2));
			LineItem.SetColor(Color);
			LineItem.LineThickness = 2.0f;
			Canvas->DrawItem(LineItem);
		}
	};

	if (HapticAsset->PatternData_L.Num() > 0 || HapticAsset->PatternData_R.Num() > 0)
	{
		if (HapticAsset->PatternData_L.Num() > 0)
		{
			DrawWaveform(HapticAsset->PatternData_L, X + MarginX, Y + MarginY, DrawWidth, DrawHeight * 0.45f, FLinearColor(0.2f, 0.6f, 1.0f));
		}

		if (HapticAsset->PatternData_R.Num() > 0)
		{
			DrawWaveform(HapticAsset->PatternData_R, X + MarginX, Y + Height * 0.55f, DrawWidth, DrawHeight * 0.45f, FLinearColor(1.0f, 0.4f, 0.2f));
		}
	}

	// Draw text info
	FString InfoText = FString::Printf(TEXT("PHF %s\n%dms\nL:%d R:%d"), 
		*HapticAsset->PHFVersion, 
		HapticAsset->FrameDuration,
		HapticAsset->PatternData_L.Num(),
		HapticAsset->PatternData_R.Num());

	FCanvasTextItem TextItem(FVector2D(X + 5, Y + 5), FText::FromString(InfoText), GEngine->GetSmallFont(), FLinearColor::White);
	TextItem.Scale = FVector2D(0.8f, 0.8f);
	TextItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TextItem);
}

bool UPICOHapticAssetThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	return Object && Object->IsA<UPICOHapticAsset>();
}
