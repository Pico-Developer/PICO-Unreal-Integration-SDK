#include "PICOSpatialExpressionVectorParameter.h"

#if USE_USD_SDK

void FExpressionVectorParameterPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (UMaterialExpressionVectorParameter* VectorParameter = Cast<UMaterialExpressionVectorParameter>(Owner))
	{
		// Create material input for the parameter
		pxr::UsdShadeMaterial UsdMaterial = pxr::UsdShadeMaterial::Get(Stage, pxr::SdfPath("/Root/Material"));
		// USD parameter names cannot contain spaces, remove them
		// Add _V suffix to distinguish from Scalar and Texture parameters
		FString ParamNameStr = VectorParameter->ParameterName.ToString().Replace(TEXT(" "), TEXT("")) + TEXT("_V");
		pxr::TfToken ParamName = pxr::TfToken(TCHAR_TO_UTF8(*ParamNameStr));
		pxr::UsdShadeInput ShadeInput = UsdMaterial.CreateInput(ParamName, pxr::SdfValueTypeNames->Color4f);
		
		// Set the default value (RGBA)
		FLinearColor DefaultValue = VectorParameter->DefaultValue;
		ShadeInput.Set(pxr::GfVec4f(DefaultValue.R, DefaultValue.G, DefaultValue.B, DefaultValue.A));
		ShadeInput.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));

		// Create the constant shader node
		{
			FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
			if (ShaderPath.IsEmpty())
			{
				return;
			}
			UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));

			// MaterialX standard node for constant color4 value
			// Parameters are treated as constants in USD export
			UsdShader.CreateIdAttr().Set(pxr::TfToken("ND_constant_color4"));

			// Create the value input and connect to material parameter
			pxr::UsdShadeInput ValueInput = UsdShader.CreateInput(pxr::TfToken("value"), pxr::SdfValueTypeNames->Color4f);
			ValueInput.ConnectToSource(ShadeInput);

			// Create outputs: main output (Color4) and individual channel outputs (Float)
			pxr::UsdShadeOutput out = UsdShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Color4f);
			ShaderOutputs.Add(0, out);
		}

		bDefinedUsdShader = true;
	}
}
#endif // USE_USD_SDK
