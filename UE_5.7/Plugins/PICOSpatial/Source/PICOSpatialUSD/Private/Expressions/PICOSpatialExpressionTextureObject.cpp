#include "PICOSpatialExpressionTextureObject.h"

#if USE_USD_SDK
bool FExpressionTextureBasePICO::IsTextureSupported(UTexture* Texture)
{
	if (!Texture)
	{
		return false;
	}

	// Check if texture is a RenderTarget - not currently supported
	if (Texture->IsA<UTexture2D>())
	{
		return true;
	}

	UE_LOG(LogPICOSpatialUSD, Warning,
		TEXT("UTexture '%s' is not supported for USD export. Only UTexture2D textures are supported."),
		*Texture->GetName());

	return false;
}

void FExpressionTextureObjectPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (UMaterialExpressionTextureObject* TextureObject = Cast<UMaterialExpressionTextureObject>(Owner))
	{
		// Check if texture is supported
		if (!IsTextureSupported(TextureObject->Texture))
		{
			UE_LOG(LogPICOSpatialUSD, Warning,
				TEXT("Failed to define shader for TextureObject node '%s': Unsupported texture type"),
				*Owner->GetName());
			return;
		}

		FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
		if (ShaderPath.IsEmpty())
		{
			return;
		}
		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));

		UsdShader.CreateIdAttr().Set(pxr::TfToken("ND_constant_filename"));

		FString TexturePath = FShaderGraphUsdConvertorPICO::GetTexturePathFromTexture(TextureObject->Texture, Stage);

		pxr::UsdShadeInput valueInput = UsdShader.CreateInput(pxr::TfToken("value"), pxr::SdfValueTypeNames->Asset);
		valueInput.Set(pxr::SdfAssetPath(""));
		if (!TexturePath.IsEmpty())
		{
			valueInput.Set(pxr::SdfAssetPath(TCHAR_TO_UTF8(*TexturePath)));
		}

		pxr::UsdShadeOutput out = UsdShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Asset);
		ShaderOutputs.Add(0, out);

		bDefinedUsdShader = true;
	}
}
#endif // USE_USD_SDK


