// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "IconImagePipeline.h"

#include "Misc/FileHelper.h"
#include "Modules/ModuleManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

#include <cmath>

// ===================================================================
// Encode / Decode
// ===================================================================
namespace
{
	IImageWrapperModule& GetImageWrapperModule()
	{
		return FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	}
}

bool IconImagePipeline::DecodeFileToRgba(const FString& SrcPath, FRgbaImage& Out)
{
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *SrcPath) || FileData.Num() == 0)
	{
		return false;
	}

	IImageWrapperModule& Module = GetImageWrapperModule();
	const EImageFormat Format = Module.DetectImageFormat(FileData.GetData(), FileData.Num());
	if (Format == EImageFormat::Invalid)
	{
		return false;
	}

	TSharedPtr<IImageWrapper> Wrapper = Module.CreateImageWrapper(Format);
	if (!Wrapper.IsValid() || !Wrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		return false;
	}

	const int32 W = Wrapper->GetWidth();
	const int32 H = Wrapper->GetHeight();
	if (W <= 0 || H <= 0)
	{
		return false;
	}

	// Force decode to standard RGBA8 (R=offset0 ... A=offset3)
	TArray<uint8> Raw;
	if (!Wrapper->GetRaw(ERGBFormat::RGBA, 8, Raw) || Raw.Num() != W * H * 4)
	{
		return false;
	}

	Out.Width  = W;
	Out.Height = H;
	Out.Pixels = MoveTemp(Raw);
	return true;
}

bool IconImagePipeline::EncodeRgbaToPngFile(const FRgbaImage& Img, const FString& DestPath)
{
	if (!Img.IsValid())
	{
		return false;
	}

	IImageWrapperModule& Module = GetImageWrapperModule();
	TSharedPtr<IImageWrapper> Wrapper = Module.CreateImageWrapper(EImageFormat::PNG);
	if (!Wrapper.IsValid())
	{
		return false;
	}

	if (!Wrapper->SetRaw(Img.Pixels.GetData(), Img.Pixels.Num(),
	                     Img.Width, Img.Height, ERGBFormat::RGBA, 8))
	{
		return false;
	}

	const TArray64<uint8>& Png = Wrapper->GetCompressed(100);
	return FFileHelper::SaveArrayToFile(Png, *DestPath);
}

// ===================================================================
// Bilinear resize / squarify
// ===================================================================
FRgbaImage IconImagePipeline::ResizeBilinear(const FRgbaImage& Src, int32 DstW, int32 DstH)
{
	FRgbaImage Dst;
	if (!Src.IsValid() || DstW <= 0 || DstH <= 0)
	{
		return Dst;
	}

	Dst.Width  = DstW;
	Dst.Height = DstH;
	Dst.Pixels.SetNumZeroed(DstW * DstH * 4);

	// Map destination pixel centers to source coordinates
	const float ScaleX = static_cast<float>(Src.Width)  / static_cast<float>(DstW);
	const float ScaleY = static_cast<float>(Src.Height) / static_cast<float>(DstH);

	for (int32 y = 0; y < DstH; ++y)
	{
		const float SrcYf = (static_cast<float>(y) + 0.5f) * ScaleY - 0.5f;
		const int32 Y0 = FMath::Clamp(FMath::FloorToInt(SrcYf), 0, Src.Height - 1);
		const int32 Y1 = FMath::Clamp(Y0 + 1, 0, Src.Height - 1);
		const float Fy = FMath::Clamp(SrcYf - static_cast<float>(Y0), 0.f, 1.f);

		for (int32 x = 0; x < DstW; ++x)
		{
			const float SrcXf = (static_cast<float>(x) + 0.5f) * ScaleX - 0.5f;
			const int32 X0 = FMath::Clamp(FMath::FloorToInt(SrcXf), 0, Src.Width - 1);
			const int32 X1 = FMath::Clamp(X0 + 1, 0, Src.Width - 1);
			const float Fx = FMath::Clamp(SrcXf - static_cast<float>(X0), 0.f, 1.f);

			const int32 O00 = Src.Offset(X0, Y0);
			const int32 O10 = Src.Offset(X1, Y0);
			const int32 O01 = Src.Offset(X0, Y1);
			const int32 O11 = Src.Offset(X1, Y1);

			const int32 DstO = Dst.Offset(x, y);
			for (int32 c = 0; c < 4; ++c)
			{
				const float Top    = FMath::Lerp(static_cast<float>(Src.Pixels[O00 + c]),
				                                  static_cast<float>(Src.Pixels[O10 + c]), Fx);
				const float Bottom = FMath::Lerp(static_cast<float>(Src.Pixels[O01 + c]),
				                                  static_cast<float>(Src.Pixels[O11 + c]), Fx);
				const float Val = FMath::Lerp(Top, Bottom, Fy);
				Dst.Pixels[DstO + c] = static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(Val), 0, 255));
			}
		}
	}

	return Dst;
}

FRgbaImage IconImagePipeline::ToSquare(const FRgbaImage& Src)
{
	FRgbaImage Out;
	if (!Src.IsValid())
	{
		return Out;
	}

	const int32 Target = (FMath::Max(Src.Width, Src.Height) > 512) ? 1024 : 512;

	// Proportionally scaled dimensions
	const float Scale = FMath::Min(static_cast<float>(Target) / Src.Width,
	                               static_cast<float>(Target) / Src.Height);
	const int32 SW = FMath::Max(1, FMath::RoundToInt(Src.Width  * Scale));
	const int32 SH = FMath::Max(1, FMath::RoundToInt(Src.Height * Scale));

	const FRgbaImage Scaled = ResizeBilinear(Src, SW, SH);

	// Center-paste onto a transparent canvas
	Out.Width  = Target;
	Out.Height = Target;
	Out.Pixels.SetNumZeroed(Target * Target * 4);

	const int32 OX = (Target - SW) / 2;
	const int32 OY = (Target - SH) / 2;

	for (int32 y = 0; y < SH; ++y)
	{
		for (int32 x = 0; x < SW; ++x)
		{
			const int32 SrcO = Scaled.Offset(x, y);
			const int32 DstO = Out.Offset(OX + x, OY + y);
			Out.Pixels[DstO + 0] = Scaled.Pixels[SrcO + 0];
			Out.Pixels[DstO + 1] = Scaled.Pixels[SrcO + 1];
			Out.Pixels[DstO + 2] = Scaled.Pixels[SrcO + 2];
			Out.Pixels[DstO + 3] = Scaled.Pixels[SrcO + 3];
		}
	}

	return Out;
}

// ===================================================================
// SDF (8SSEDT) -- 1:1 port from Kotlin SdfGenerator
// ===================================================================
namespace
{
	constexpr float kSdfSentinel = 1000000.0f;

	FORCEINLINE float SdfClamp(float V, float MinV, float MaxV)
	{
		return (V < MinV) ? MinV : ((V > MaxV) ? MaxV : V);
	}

	/** Equivalent to Kotlin SdfContext. All SoA buffers stored flat. */
	struct FSdfContext
	{
		int32 W;
		int32 H;
		int32 PixelCount;
		TArray<float> Alphas;
		TArray<float> Distances;
		TArray<float> GradXs;
		TArray<float> GradYs;
		TArray<int32> DXs;
		TArray<int32> DYs;

		FSdfContext(int32 InW, int32 InH)
			: W(InW), H(InH), PixelCount(InW * InH)
		{
			Alphas.SetNumZeroed(PixelCount);
			Distances.SetNumZeroed(PixelCount);
			GradXs.SetNumZeroed(PixelCount);
			GradYs.SetNumZeroed(PixelCount);
			DXs.SetNumZeroed(PixelCount);
			DYs.SetNumZeroed(PixelCount);
		}

		float ApproximateEdgeDelta(float GxIn, float GyIn, float A) const
		{
			if (GxIn == 0.0f || GyIn == 0.0f)
			{
				return 0.5f - A;
			}
			const float Length = FMath::Sqrt(GxIn * GxIn + GyIn * GyIn);
			float Gx = FMath::Abs(GxIn / Length);
			float Gy = FMath::Abs(GyIn / Length);
			if (Gx < Gy)
			{
				const float Temp = Gx;
				Gx = Gy;
				Gy = Temp;
			}

			const float A1 = 0.5f * Gy / Gx;
			if (A < A1)
			{
				return 0.5f * (Gx + Gy) - FMath::Sqrt(2.0f * Gx * Gy * A);
			}
			if (A < (1.0f - A1))
			{
				return (0.5f - A) * Gx;
			}
			return -0.5f * (Gx + Gy) + FMath::Sqrt(2.0f * Gx * Gy * (1.0f - A));
		}

		void ComputeEdgeGradients()
		{
			const float Sqrt2 = FMath::Sqrt(2.0f);
			for (int32 y = 1; y < H - 1; ++y)
			{
				for (int32 x = 1; x < W - 1; ++x)
				{
					const int32 Idx = y * W + x;
					const float A = Alphas[Idx];
					if (A > 0.0f && A < 1.0f)
					{
						const int32 IdxTL = (y - 1) * W + (x - 1);
						const int32 IdxTR = (y - 1) * W + (x + 1);
						const int32 IdxBL = (y + 1) * W + (x - 1);
						const int32 IdxBR = (y + 1) * W + (x + 1);
						const int32 IdxL  = y * W + (x - 1);
						const int32 IdxR  = y * W + (x + 1);
						const int32 IdxT  = (y - 1) * W + x;
						const int32 IdxB  = (y + 1) * W + x;

						const float G = -Alphas[IdxTL] - Alphas[IdxBL] + Alphas[IdxTR] + Alphas[IdxBR];
						const float Gx = G + (Alphas[IdxR] - Alphas[IdxL]) * Sqrt2;
						const float Gy = G + (Alphas[IdxB] - Alphas[IdxT]) * Sqrt2;
						const float Len = FMath::Sqrt(Gx * Gx + Gy * Gy);
						if (Len > 0)
						{
							GradXs[Idx] = Gx / Len;
							GradYs[Idx] = Gy / Len;
						}
					}
				}
			}
		}

		void UpdateDistance(int32 X, int32 Y, int32 OX, int32 OY)
		{
			const int32 PIdx  = Y * W + X;
			const int32 NIdx  = (Y + OY) * W + (X + OX);
			const int32 ClIdx = (Y + OY - DYs[NIdx]) * W + (X + OX - DXs[NIdx]);

			const float ClAlpha = Alphas[ClIdx];
			if (ClAlpha == 0.0f || ClIdx == PIdx)
			{
				return;
			}

			const int32 NDX = DXs[NIdx] - OX;
			const int32 NDY = DYs[NIdx] - OY;
			const float Dist = FMath::Sqrt(static_cast<float>(NDX * NDX + NDY * NDY))
				+ ApproximateEdgeDelta(static_cast<float>(NDX), static_cast<float>(NDY), ClAlpha);

			if (Dist < Distances[PIdx])
			{
				Distances[PIdx] = Dist;
				DXs[PIdx] = NDX;
				DYs[PIdx] = NDY;
			}
		}

		void GenerateDistanceTransform()
		{
			for (int32 i = 0; i < PixelCount; ++i)
			{
				DXs[i] = 0;
				DYs[i] = 0;
				const float A = Alphas[i];
				if (A <= 0.0f)
				{
					Distances[i] = kSdfSentinel;
				}
				else if (A < 1.0f)
				{
					Distances[i] = ApproximateEdgeDelta(GradXs[i], GradYs[i], A);
				}
				else
				{
					Distances[i] = 0.0f;
				}
			}

			// 8SSED Passes
			for (int32 y = 1; y < H; ++y)
			{
				for (int32 x = 0; x < W; ++x)
				{
					if (Distances[y * W + x] <= 0.0f) continue;
					if (x > 0) UpdateDistance(x, y, -1, 0);
					if (x > 0) UpdateDistance(x, y, -1, -1);
					UpdateDistance(x, y, 0, -1);
					if (x < W - 1) UpdateDistance(x, y, 1, -1);
				}
				for (int32 x = W - 2; x >= 0; --x)
				{
					if (Distances[y * W + x] > 0.0f) UpdateDistance(x, y, 1, 0);
				}
			}
			for (int32 y = H - 2; y >= 0; --y)
			{
				for (int32 x = W - 1; x >= 0; --x)
				{
					if (Distances[y * W + x] <= 0.0f) continue;
					if (x < W - 1) UpdateDistance(x, y, 1, 0);
					if (x < W - 1) UpdateDistance(x, y, 1, 1);
					UpdateDistance(x, y, 0, 1);
					if (x > 0) UpdateDistance(x, y, -1, 1);
				}
				for (int32 x = 1; x < W; ++x)
				{
					if (Distances[y * W + x] > 0.0f) UpdateDistance(x, y, -1, 0);
				}
			}
		}

		void PostProcess(float MaxDist)
		{
			for (int32 i = 0; i < PixelCount; ++i)
			{
				const int32 PdX = DXs[i];
				const int32 PdY = DYs[i];
				if ((PdX == 0 && PdY == 0) || Distances[i] >= MaxDist) continue;

				const int32 x = i % W;
				const int32 y = i / W;
				const int32 ClIdx = (y - PdY) * W + (x - PdX);
				const float ClGradX = GradXs[ClIdx];
				const float ClGradY = GradYs[ClIdx];
				if (ClGradX == 0.0f && ClGradY == 0.0f) continue;

				const float Df = ApproximateEdgeDelta(ClGradX, ClGradY, Alphas[ClIdx]);
				const float T = static_cast<float>(PdY) * ClGradX - static_cast<float>(PdX) * ClGradY;
				const float U = -Df * ClGradX + T * ClGradY;
				const float V = -Df * ClGradY - T * ClGradX;

				if (FMath::Abs(U) <= 0.5f && FMath::Abs(V) <= 0.5f)
				{
					Distances[i] = FMath::Sqrt((PdX + U) * (PdX + U) + (PdY + V) * (PdY + V));
				}
			}
		}
	};
}

FRgbaImage IconImagePipeline::GenerateSdf(const FRgbaImage& Src,
                                          float MaxInside, float MaxOutside,
                                          float PostProcessDistance,
                                          EIconSdfRgbMode RgbMode)
{
	FRgbaImage Dst;
	if (!Src.IsValid())
	{
		return Dst;
	}

	const int32 W = Src.Width;
	const int32 H = Src.Height;
	const int32 PixelCount = W * H;

	Dst.Width  = W;
	Dst.Height = H;
	Dst.Pixels.SetNumZeroed(PixelCount * 4);

	FSdfContext Context(W, H);

	// Pass 1: interior distance
	if (MaxInside > 0.0f)
	{
		for (int32 i = 0; i < PixelCount; ++i)
		{
			const uint8 AlphaByte = Src.Pixels[i * 4 + 3];
			Context.Alphas[i] = 1.0f - (AlphaByte / 255.0f);
		}
		Context.ComputeEdgeGradients();
		Context.GenerateDistanceTransform();
		if (PostProcessDistance > 0.0f) Context.PostProcess(PostProcessDistance);

		const float Scale = 1.0f / MaxInside;
		for (int32 i = 0; i < PixelCount; ++i)
		{
			const float Dist = SdfClamp(Context.Distances[i] * Scale, 0.0f, 1.0f);
			Dst.Pixels[i * 4 + 3] = static_cast<uint8>(Dist * 255.0f);
		}
	}

	// Pass 2: exterior distance
	if (MaxOutside > 0.0f)
	{
		for (int32 i = 0; i < PixelCount; ++i)
		{
			const uint8 AlphaByte = Src.Pixels[i * 4 + 3];
			Context.Alphas[i] = AlphaByte / 255.0f;
		}
		Context.ComputeEdgeGradients();
		Context.GenerateDistanceTransform();
		if (PostProcessDistance > 0.0f) Context.PostProcess(PostProcessDistance);

		const float Scale = 1.0f / MaxOutside;
		for (int32 i = 0; i < PixelCount; ++i)
		{
			const float CurrentA = (MaxInside > 0.0f) ? (Dst.Pixels[i * 4 + 3] / 255.0f) : 1.0f;
			const float Dist = SdfClamp(Context.Distances[i] * Scale, 0.0f, 1.0f);

			const float FinalA = (MaxInside > 0.0f)
				? (0.5f + (CurrentA - Dist) * 0.5f)
				: (1.0f - Dist);

			Dst.Pixels[i * 4 + 3] = static_cast<uint8>(SdfClamp(FinalA, 0.0f, 1.0f) * 255.0f);
		}
	}

	// RGB fill
	for (int32 i = 0; i < PixelCount; ++i)
	{
		const int32 O = i * 4;
		switch (RgbMode)
		{
		case EIconSdfRgbMode::Distance:
		{
			const uint8 A = Dst.Pixels[O + 3];
			Dst.Pixels[O + 0] = A;
			Dst.Pixels[O + 1] = A;
			Dst.Pixels[O + 2] = A;
			break;
		}
		case EIconSdfRgbMode::Source:
			Dst.Pixels[O + 0] = Src.Pixels[O + 0];
			Dst.Pixels[O + 1] = Src.Pixels[O + 1];
			Dst.Pixels[O + 2] = Src.Pixels[O + 2];
			break;
		default:
		{
			const uint8 Color = (RgbMode == EIconSdfRgbMode::White) ? 255 : 0;
			Dst.Pixels[O + 0] = Color;
			Dst.Pixels[O + 1] = Color;
			Dst.Pixels[O + 2] = Color;
			break;
		}
		}
	}

	return Dst;
}

FRgbaImage IconImagePipeline::GenerateSdfDefault(const FRgbaImage& Src)
{
	const int32 W = Src.Width;
	const float MaxInside = (W >= 1024) ? 32.0f : ((W >= 512) ? 16.0f : (W / 32.0f));
	return GenerateSdf(Src, MaxInside, 0.0f, 0.0f, EIconSdfRgbMode::Distance);
}

// ===================================================================
// Layer compositing (straight-alpha over)
// ===================================================================
FRgbaImage IconImagePipeline::CompositeLayers(const TArray<FRgbaImage>& Layers,
                                              const TArray<float>& Opacities,
                                              int32 OutSize)
{
	FRgbaImage Dst;
	if (OutSize <= 0)
	{
		return Dst;
	}

	Dst.Width  = OutSize;
	Dst.Height = OutSize;
	Dst.Pixels.SetNumZeroed(OutSize * OutSize * 4);   // Transparent background

	const int32 PixelCount = OutSize * OutSize;

	// Bottom-to-top (index 0 = bottom) per-layer over compositing
	for (int32 LayerIdx = 0; LayerIdx < Layers.Num(); ++LayerIdx)
	{
		const FRgbaImage& Layer = Layers[LayerIdx];
		if (!Layer.IsValid())
		{
			continue;
		}

		// Resize to OutSize if dimensions differ
		const FRgbaImage* SrcPtr = &Layer;
		FRgbaImage Resized;
		if (Layer.Width != OutSize || Layer.Height != OutSize)
		{
			Resized = ResizeBilinear(Layer, OutSize, OutSize);
			SrcPtr = &Resized;
		}

		const float Opacity = Opacities.IsValidIndex(LayerIdx)
			? FMath::Clamp(Opacities[LayerIdx], 0.0f, 1.0f) : 1.0f;

		for (int32 i = 0; i < PixelCount; ++i)
		{
			const int32 O = i * 4;
			const float Sa = (SrcPtr->Pixels[O + 3] / 255.0f) * Opacity;
			if (Sa <= 0.0f)
			{
				continue;
			}

			const float Sr = SrcPtr->Pixels[O + 0] / 255.0f;
			const float Sg = SrcPtr->Pixels[O + 1] / 255.0f;
			const float Sb = SrcPtr->Pixels[O + 2] / 255.0f;

			const float Da = Dst.Pixels[O + 3] / 255.0f;
			const float Dr = Dst.Pixels[O + 0] / 255.0f;
			const float Dg = Dst.Pixels[O + 1] / 255.0f;
			const float Db = Dst.Pixels[O + 2] / 255.0f;

			// straight-alpha over
			const float OutA = Sa + Da * (1.0f - Sa);
			if (OutA <= 0.0f)
			{
				continue;
			}
			const float OutR = (Sr * Sa + Dr * Da * (1.0f - Sa)) / OutA;
			const float OutG = (Sg * Sa + Dg * Da * (1.0f - Sa)) / OutA;
			const float OutB = (Sb * Sa + Db * Da * (1.0f - Sa)) / OutA;

			Dst.Pixels[O + 0] = static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(OutR * 255.0f), 0, 255));
			Dst.Pixels[O + 1] = static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(OutG * 255.0f), 0, 255));
			Dst.Pixels[O + 2] = static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(OutB * 255.0f), 0, 255));
			Dst.Pixels[O + 3] = static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(OutA * 255.0f), 0, 255));
		}
	}

	return Dst;
}

// ===================================================================
// BCP-47 → Android values qualifier
// ===================================================================
FString IconImagePipeline::Bcp47ToAndroidQualifier(const FString& Bcp47Tag)
{
	const FString Tag = Bcp47Tag.TrimStartAndEnd();
	if (Tag.IsEmpty())
	{
		return FString();   // Default base
	}

	// Default base locale → values (empty suffix)
	if (Tag.Equals(TEXT("en-US"), ESearchCase::IgnoreCase))
	{
		return FString();
	}

	FString Lang;
	FString Region;
	if (!Tag.Split(TEXT("-"), &Lang, &Region))
	{
		// Single-language tag (e.g. "ar")
		return Tag.ToLower();
	}

	Lang = Lang.ToLower();

	// Numeric UN M.49 region code (e.g. es-419) → b+lang+REGION
	if (Region.Len() > 0 && FChar::IsDigit(Region[0]))
	{
		return FString::Printf(TEXT("b+%s+%s"), *Lang, *Region);
	}

	// Standard region → lang-rREGION (region uppercased)
	return FString::Printf(TEXT("%s-r%s"), *Lang, *Region.ToUpper());
}
