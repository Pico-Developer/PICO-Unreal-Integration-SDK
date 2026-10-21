// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * RGBA8 pixel buffer (channel order fixed as R,G,B,A, row-major).
 * The entire image pipeline uses RGBA8 internally and never uses FColor(BGRA) or locked Mips,
 * to avoid channel and color issues caused by UE texture BGRA / sRGB / block compression.
 */
struct FRgbaImage
{
	TArray<uint8> Pixels;   // Size == Width * Height * 4
	int32 Width  = 0;
	int32 Height = 0;

	bool IsValid() const
	{
		return Width > 0 && Height > 0 && Pixels.Num() == Width * Height * 4;
	}

	FORCEINLINE int32 Offset(int32 X, int32 Y) const { return (Y * Width + X) * 4; }
};

/** RGB channel fill mode for SDF output (matches source Kotlin RGBFillMode). */
enum class EIconSdfRgbMode : uint8
{
	White,
	Black,
	Distance,
	Source,
};

/**
 * Layered icon image processing pipeline (ported from spatial-plugin).
 * All functions are stateless statics.
 */
namespace IconImagePipeline
{
	// ---- Encode / Decode ----
	/** Decode an image file from disk to RGBA8 using ImageWrapper. Supports PNG/JPEG/BMP etc. */
	bool DecodeFileToRgba(const FString& SrcPath, FRgbaImage& Out);

	/** Encode an RGBA8 buffer to PNG and write to disk. */
	bool EncodeRgbaToPngFile(const FRgbaImage& Img, const FString& DestPath);

	// ---- Resize / Squarify ----
	/** Bilinear resize (alpha interpolated together, straight non-premultiplied). */
	FRgbaImage ResizeBilinear(const FRgbaImage& Src, int32 DstW, int32 DstH);

	/** Force 1:1 square: uses 1024 if either side > 512, otherwise 512. Centered proportional scale with transparent padding. */
	FRgbaImage ToSquare(const FRgbaImage& Src);

	// ---- SDF (8SSEDT, ported from Kotlin SdfGenerator) ----
	FRgbaImage GenerateSdf(const FRgbaImage& Src,
	                       float MaxInside, float MaxOutside,
	                       float PostProcessDistance,
	                       EIconSdfRgbMode RgbMode);

	/** Generate SDF with plugin-default parameters: maxInside=(w>=1024?32:w>=512?16:w/32), maxOutside=0, rgbMode=Distance. */
	FRgbaImage GenerateSdfDefault(const FRgbaImage& Src);

	// ---- Layer compositing (straight-alpha over, for ic_spatial_launcher.png) ----
	/** Layers passed in bottom-to-top order (index 0 = background). Composites with Opacities applied, outputs an OutSize square. */
	FRgbaImage CompositeLayers(const TArray<FRgbaImage>& Layers,
	                           const TArray<float>& Opacities,
	                           int32 OutSize);

	// ---- Localization: BCP-47 → Android values qualifier ----
	/** en-US → "" (default base); zh-CN → "zh-rCN"; ja-JP → "ja-rJP"; ko-KR → "ko-rKR". */
	FString Bcp47ToAndroidQualifier(const FString& Bcp47Tag);
}
