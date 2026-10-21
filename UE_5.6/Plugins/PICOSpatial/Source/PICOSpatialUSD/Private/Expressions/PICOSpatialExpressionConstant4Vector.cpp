#include "PICOSpatialExpressionConstant4Vector.h"

#if USE_USD_SDK

void FExpressionConstant4VectorPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (UMaterialExpressionConstant4Vector* Constant4 = Cast<UMaterialExpressionConstant4Vector>(Owner))
	{
		FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
		if (ShaderPath.IsEmpty())
		{
			return;
		}
		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
		
		// MaterialX standard node for constant color4 value
		UsdShader.CreateIdAttr().Set(pxr::TfToken("ND_constant_color4"));

		// Create the value input with RGBA components
		pxr::UsdShadeInput valueInput = UsdShader.CreateInput(pxr::TfToken("value"), pxr::SdfValueTypeNames->Color4f);
		valueInput.Set(pxr::GfVec4f(Constant4->Constant.R, Constant4->Constant.G, Constant4->Constant.B, Constant4->Constant.A));
		valueInput.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));

		// Create outputs: main output (Color4) and individual channel outputs (Float)
		pxr::UsdShadeOutput out = UsdShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Color4f);
		ShaderOutputs.Add(0, out);

		bDefinedUsdShader = true;
	}
}
#endif // USE_USD_SDK

