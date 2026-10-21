#include "PICOSpatialExpressionMFCameraDirection.h"

#include "Materials/MaterialExpressionMaterialFunctionCall.h"

#if USE_USD_SDK
void FExpressionMFCameraDirectionPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (!Cast<UMaterialExpressionMaterialFunctionCall>(Owner))
	{
		return;
	}

	FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/CameraDirection"));
	if (ShaderPath.IsEmpty())
	{
		return;
	}

	UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
	UsdShader.CreateIdAttr().Set(pxr::TfToken("ND_cameradirection"));

	pxr::UsdShadeOutput Out = UsdShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float3);

	ShaderOutputs.Add(0, Out);
	bDefinedUsdShader = true;
}
#endif // USE_USD_SDK
