#include "PICOSpatialExpressionScalarParameter.h"

#if USE_USD_SDK

void FExpressionScalarParameterPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (UMaterialExpressionScalarParameter* ScalarParameter = Cast<UMaterialExpressionScalarParameter>(Owner))
	{
		pxr::UsdShadeMaterial UsdMaterial = pxr::UsdShadeMaterial::Get(Stage, pxr::SdfPath("/Root/Material"));
		// USD parameter names cannot contain spaces, remove them
		// Add _S suffix to distinguish from Vector and Texture parameters
		FString ParamNameStr = ScalarParameter->ParameterName.ToString().Replace(TEXT(" "), TEXT("")) + TEXT("_S");
		pxr::TfToken ParamName = pxr::TfToken(TCHAR_TO_UTF8(*ParamNameStr));
		pxr::UsdShadeInput ShadeInput = UsdMaterial.CreateInput(ParamName, pxr::SdfValueTypeNames->Float);
		ShadeInput.Set(ScalarParameter->DefaultValue);

		{
			FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
			if (ShaderPath.IsEmpty())
			{
				return;
			}
			UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));

			// MaterialX standard node for constant float value
			// Parameters are treated as constants in USD export
			UsdShader.CreateIdAttr().Set(pxr::TfToken("ND_constant_float"));

			// Create the value input with the parameter's default value
			pxr::UsdShadeInput ValueInput = UsdShader.CreateInput(pxr::TfToken("value"), pxr::SdfValueTypeNames->Float);
			ValueInput.ConnectToSource(ShadeInput);

			// Create the output
			pxr::UsdShadeOutput out = UsdShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float);
			ShaderOutputs.Add(0, out);
		}

		bDefinedUsdShader = true;
	}
}
#endif // USE_USD_SDK


