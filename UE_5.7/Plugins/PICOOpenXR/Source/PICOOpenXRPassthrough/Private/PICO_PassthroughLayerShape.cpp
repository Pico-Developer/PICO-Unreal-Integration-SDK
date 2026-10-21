// Fill out your copyright notice in the Description page of Project Settings.


#include "PICO_PassthroughLayerShape.h"
#include "Curves/CurveLinearColor.h"
#include "openxr/openxr.h"

const FName FFullScreenLayerShapePICO::ShapeName = FName("FullScreenLayer");
const FName FCustomShapeLayerShapePICO::ShapeName = FName("CustomShapeLayer");

TArray<uint8> FVSTEdgeStyleParametersPICO::GenerateColorMapData(EColorMapTypePICO InColorMapType, const TArray<FLinearColor>& InColorMapGradient)
{
	switch (InColorMapType) {
	case EColorMapTypePICO::ColorMapType_GrayscaleToColor: {
		TArray<uint8> NewColorMapData = GenerateMonoBrightnessContrastPosterizeMap();
		return GenerateMonoToRGBA(InColorMapGradient, NewColorMapData);
	}
	case EColorMapTypePICO::ColorMapType_Grayscale:
		return GenerateMonoBrightnessContrastPosterizeMap();
	case EColorMapTypePICO::ColorMapType_ColorAdjustment:
		return GenerateBrightnessContrastSaturationColorMap();
	default:
		return TArray<uint8>();
	}
}

TArray<uint8> FVSTEdgeStyleParametersPICO::GenerateMonoToRGBA(const TArray<FLinearColor>& InGradient, const TArray<uint8>& InColorMapData)
{
	TArray<uint8> NewColorMapData;
	FInterpCurveLinearColor InterpCurve;
	const uint32 TotalEntries = 256;

	for (int32 Index = 0; Index < InGradient.Num(); ++Index)
	{
		InterpCurve.AddPoint(Index, (InGradient[Index] * ColorScale) + ColorOffset);
	}

	NewColorMapData.SetNum(TotalEntries * sizeof(XrColor4f));
	uint8* Dest = NewColorMapData.GetData();
	for (int32 Index = 0; Index < TotalEntries; ++Index)
	{
		FLinearColor LinearColor = InterpCurve.Eval(InColorMapData[Index]);
		const XrColor4f Color = XrColor4f{ LinearColor.R, LinearColor.G, LinearColor.B, LinearColor.A };
		FMemory::Memcpy(Dest, &Color, sizeof(Color));
		Dest += sizeof(XrColor4f);
	}
	return NewColorMapData;
}

TArray<uint8> FVSTEdgeStyleParametersPICO::GenerateMonoBrightnessContrastPosterizeMap()
{
	TArray<uint8> NewColorMapData;
	const int32 TotalEntries = 256;
	NewColorMapData.SetNum(TotalEntries * sizeof(uint8));
	for (int32 Index = 0; Index < TotalEntries; ++Index)
	{
		float Alpha = ((float)Index / TotalEntries);
		float ContrastFactor = Contrast + 1.0;
		Alpha = (Alpha - 0.5) * ContrastFactor + 0.5 + Brightness;

		if (Posterize > 0.0f)
		{
			const float PosterizationBase = 50.0f;
			float FinalPosterize = (FMath::Pow(PosterizationBase, Posterize) - 1.0) / (PosterizationBase - 1.0);
			Alpha = FMath::RoundToFloat(Alpha / FinalPosterize) * FinalPosterize;
		}

		NewColorMapData[Index] = (uint8)(FMath::Min(FMath::Max(Alpha, 0.0f), 1.0f) * 255.0f);
	}
	return NewColorMapData;
}

TArray<uint8> FVSTEdgeStyleParametersPICO::GenerateBrightnessContrastSaturationColorMap()
{
	TArray<uint8> NewColorMapData;
	NewColorMapData.SetNum(3 * sizeof(float));
	float newB = Brightness * 100.0f;
	float newC = Contrast + 1.0f;
	float newS = Saturation + 1.0f;

	uint8* Dest = NewColorMapData.GetData();
	FMemory::Memcpy(Dest, &newB, sizeof(float));
	Dest += sizeof(float);
	FMemory::Memcpy(Dest, &newC, sizeof(float));
	Dest += sizeof(float);
	FMemory::Memcpy(Dest, &newS, sizeof(float));

	return NewColorMapData;
}

PassthroughColorMapType FVSTEdgeStyleParametersPICO::GetPICOColorMapType(EColorMapTypePICO InColorMapType)
{
	switch (InColorMapType) {
	case EColorMapTypePICO::ColorMapType_GrayscaleToColor:
		return PassthroughColorMapType::MonoToRgba;
	case EColorMapTypePICO::ColorMapType_Grayscale:
		return PassthroughColorMapType::MonoToMono;
	case EColorMapTypePICO::ColorMapType_ColorAdjustment:
		return PassthroughColorMapType::BrightnessContrastSaturation;
	default:
		return PassthroughColorMapType::NoneStyle;
	}
}
