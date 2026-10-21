// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "RHI.h"
#include "RenderResource.h"
#include "ShaderParameters.h"
#include "Shader.h"
#include "GlobalShader.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"

class FDisplayMappingPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FDisplayMappingPixelShader, Global);
public:

	class FArraySource : SHADER_PERMUTATION_BOOL("DISPLAY_MAPPING_PS_FROM_ARRAY");
	class FLinearInput : SHADER_PERMUTATION_BOOL("DISPLAY_MAPPING_INPUT_IS_LINEAR");
	using FPermutationDomain = TShaderPermutationDomain<FArraySource, FLinearInput>;

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}

	FDisplayMappingPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FGlobalShader(Initializer)
	{
		OutputDevice.Bind(Initializer.ParameterMap, TEXT("OutputDevice"));
		OutputGamut.Bind(Initializer.ParameterMap, TEXT("OutputGamut"));
		SceneTexture.Bind(Initializer.ParameterMap, TEXT("SceneTexture"));
		SceneSampler.Bind(Initializer.ParameterMap, TEXT("SceneSampler"));
		TextureToOutputGamutMatrix.Bind(Initializer.ParameterMap, TEXT("TextureToOutputGamutMatrix"));
	}
	FDisplayMappingPixelShader() = default;

	static FMatrix44f GamutToXYZMatrix(EDisplayColorGamut ColorGamut)
	{
		static const FMatrix44f sRGB_2_XYZ_MAT(
			FVector3f(0.4124564, 0.3575761, 0.1804375),
			FVector3f(0.2126729, 0.7151522, 0.0721750),
			FVector3f(0.0193339, 0.1191920, 0.9503041),
			FVector3f(0, 0, 0)
		);

		static const FMatrix44f Rec2020_2_XYZ_MAT(
			FVector3f(0.6369736, 0.1446172, 0.1688585),
			FVector3f(0.2627066, 0.6779996, 0.0592938),
			FVector3f(0.0000000, 0.0280728, 1.0608437),
			FVector3f(0, 0, 0)
		);

		static const FMatrix44f P3D65_2_XYZ_MAT(
			FVector3f(0.4865906, 0.2656683, 0.1981905),
			FVector3f(0.2289838, 0.6917402, 0.0792762),
			FVector3f(0.0000000, 0.0451135, 1.0438031),
			FVector3f(0, 0, 0)
		);
		switch (ColorGamut)
		{
		case EDisplayColorGamut::sRGB_D65: return sRGB_2_XYZ_MAT;
		case EDisplayColorGamut::Rec2020_D65: return Rec2020_2_XYZ_MAT;
		case EDisplayColorGamut::DCIP3_D65: return P3D65_2_XYZ_MAT;
		default:
			checkNoEntry();
			return FMatrix44f::Identity;
		}

	}

	static FMatrix44f XYZToGamutMatrix(EDisplayColorGamut ColorGamut)
	{
		static const FMatrix44f XYZ_2_sRGB_MAT(
			FVector3f(3.2409699419, -1.5373831776, -0.4986107603),
			FVector3f(-0.9692436363, 1.8759675015, 0.0415550574),
			FVector3f(0.0556300797, -0.2039769589, 1.0569715142),
			FVector3f(0, 0, 0)
		);

		static const FMatrix44f XYZ_2_Rec2020_MAT(
			FVector3f(1.7166084, -0.3556621, -0.2533601),
			FVector3f(-0.6666829, 1.6164776, 0.0157685),
			FVector3f(0.0176422, -0.0427763, 0.94222867),
			FVector3f(0, 0, 0)
		);

		static const FMatrix44f XYZ_2_P3D65_MAT(
			FVector3f(2.4933963, -0.9313459, -0.4026945),
			FVector3f(-0.8294868, 1.7626597, 0.0236246),
			FVector3f(0.0358507, -0.0761827, 0.9570140),
			FVector3f(0, 0, 0)
		);

		switch (ColorGamut)
		{
		case EDisplayColorGamut::sRGB_D65: return XYZ_2_sRGB_MAT;
		case EDisplayColorGamut::Rec2020_D65: return XYZ_2_Rec2020_MAT;
		case EDisplayColorGamut::DCIP3_D65: return XYZ_2_P3D65_MAT;
		default:
			checkNoEntry();
			return FMatrix44f::Identity;
		}

	}

	void SetParameters(FRHIBatchedShaderParameters& BatchedParameters, EDisplayOutputFormat DisplayOutputFormat, EDisplayColorGamut DisplayColorGamut, EDisplayColorGamut TextureColorGamut, FRHITexture* SceneTextureRHI, bool bSameSize)
	{
		int32 OutputDeviceValue = (int32)DisplayOutputFormat;
		int32 OutputGamutValue = (int32)DisplayColorGamut;

		SetShaderValue(BatchedParameters, OutputDevice, OutputDeviceValue);
		SetShaderValue(BatchedParameters, OutputGamut, OutputGamutValue);

		const FMatrix44f TextureGamutMatrixToXYZ = GamutToXYZMatrix(TextureColorGamut);
		const FMatrix44f XYZToDisplayMatrix = XYZToGamutMatrix(DisplayColorGamut);
		// note: we use mul(m,v) instead of mul(v,m) in the shaders for color conversions which is why matrix multiplication is reversed compared to what we usually do
		const FMatrix44f CombinedMatrix = XYZToDisplayMatrix * TextureGamutMatrixToXYZ;

		SetShaderValue(BatchedParameters, TextureToOutputGamutMatrix, CombinedMatrix);

		if (bSameSize)
		{
			SetTextureParameter(BatchedParameters, SceneTexture, SceneSampler, TStaticSamplerState<SF_Point>::GetRHI(), SceneTextureRHI);
		}
		else
		{
			SetTextureParameter(BatchedParameters, SceneTexture, SceneSampler, TStaticSamplerState<SF_Bilinear>::GetRHI(), SceneTextureRHI);
		}
	}

	static const TCHAR* GetSourceFilename()
	{
		return TEXT("/Engine/Private/DisplayMappingPixelShader.usf");
	}

	static const TCHAR* GetFunctionName()
	{
		return TEXT("DisplayMappingPS");
	}

private:
	LAYOUT_FIELD(FShaderParameter, OutputDevice);
	LAYOUT_FIELD(FShaderParameter, OutputGamut);
	LAYOUT_FIELD(FShaderParameter, TextureToOutputGamutMatrix);
	LAYOUT_FIELD(FShaderResourceParameter, SceneTexture);
	LAYOUT_FIELD(FShaderResourceParameter, SceneSampler);
};

/**
* A pixel shader for rendering a textured screen element.
*/
class FPICOCubemapPS : public FGlobalShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FPICOCubemapPS, Global, PICOOPENXRHMD_API);
public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return true; }

	FPICOCubemapPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FGlobalShader(Initializer)
	{
		InTexture.Bind(Initializer.ParameterMap, TEXT("InTextureCube"), SPF_Mandatory);
		InTextureSampler.Bind(Initializer.ParameterMap, TEXT("InTextureSampler"));
		InFaceIndexParameter.Bind(Initializer.ParameterMap, TEXT("CubeFaceIndex"));
	}
	FPICOCubemapPS() {}

	void SetParameters(FRHIBatchedShaderParameters& BatchedParameters, const FTexture* Texture, int FaceIndex)
	{
		SetTextureParameter(BatchedParameters, InTexture, InTextureSampler, Texture);
		SetShaderValue(BatchedParameters, InFaceIndexParameter, FaceIndex);
	}

	void SetParameters(FRHIBatchedShaderParameters& BatchedParameters, FRHISamplerState* SamplerStateRHI, FRHITexture* TextureRHI, int FaceIndex)
	{
		SetTextureParameter(BatchedParameters, InTexture, InTextureSampler, SamplerStateRHI, TextureRHI);
		SetShaderValue(BatchedParameters, InFaceIndexParameter, FaceIndex);
	}

private:
	LAYOUT_FIELD(FShaderResourceParameter, InTexture);
	LAYOUT_FIELD(FShaderResourceParameter, InTextureSampler);
	LAYOUT_FIELD(FShaderParameter, InFaceIndexParameter);
};