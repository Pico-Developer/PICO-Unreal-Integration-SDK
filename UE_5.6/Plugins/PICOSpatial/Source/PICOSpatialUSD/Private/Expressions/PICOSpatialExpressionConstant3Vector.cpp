#include "PICOSpatialExpressionConstant3Vector.h"

#if USE_USD_SDK

void FExpressionConstant3VectorPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (UMaterialExpressionConstant3Vector* Constant3 = Cast<UMaterialExpressionConstant3Vector>(Owner))
	{
		FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
		if (ShaderPath.IsEmpty())
		{
			return;
		}
		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
		
		// MaterialX standard node for constant color3 value
		UsdShader.CreateIdAttr().Set(pxr::TfToken("ND_constant_color3"));

		// Create the value input with RGB components
		pxr::UsdShadeInput valueInput = UsdShader.CreateInput(pxr::TfToken("value"), pxr::SdfValueTypeNames->Color3f);
		valueInput.Set(pxr::GfVec3f(Constant3->Constant.R, Constant3->Constant.G, Constant3->Constant.B));
		valueInput.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));

		// Create outputs: main output (Color3) and individual channel outputs (Float)
		pxr::UsdShadeOutput out = UsdShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Color3f);
		ShaderOutputs.Add(0, out);

		bDefinedUsdShader = true;
	}
}
#endif // USE_USD_SDK

