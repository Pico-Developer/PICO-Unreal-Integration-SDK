#include "PICOSpatialExpressionConstant.h"

#if USE_USD_SDK

void FExpressionConstantPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (UMaterialExpressionConstant* Constant = Cast<UMaterialExpressionConstant>(Owner))
	{
		FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
		if (ShaderPath.IsEmpty())
		{
			return;
		}
		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
		
		// MaterialX standard node for constant float value
		UsdShader.CreateIdAttr().Set(pxr::TfToken("ND_constant_float"));

		// Create the value input
		pxr::UsdShadeInput valueInput = UsdShader.CreateInput(pxr::TfToken("value"), pxr::SdfValueTypeNames->Float);
		valueInput.Set(Constant->R);

		// Create the output
		pxr::UsdShadeOutput out = UsdShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float);
		ShaderOutputs.Add(0, out);

		bDefinedUsdShader = true;
	}
}
#endif // USE_USD_SDK

