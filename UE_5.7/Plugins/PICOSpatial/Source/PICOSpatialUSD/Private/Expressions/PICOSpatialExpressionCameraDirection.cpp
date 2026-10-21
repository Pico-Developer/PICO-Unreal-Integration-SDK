#include "PICOSpatialExpressionCameraDirection.h"

#include "Materials/MaterialExpressionCameraVectorWS.h"

#if USE_USD_SDK
void FExpressionCameraDirectionPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (Cast<UMaterialExpressionCameraVectorWS>(Owner))
	{
		FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/ViewDirection"));
		if (ShaderPath.IsEmpty())
		{
			return;
		}

		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
		UsdShader.CreateIdAttr().Set(pxr::TfToken("ND__viewdirection_vector3"));

		pxr::UsdShadeInput Space = UsdShader.CreateInput(pxr::TfToken("space"), pxr::SdfValueTypeNames->String);
		Space.Set(std::string("world"));

		pxr::UsdShadeOutput Out = UsdShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float3);

		ShaderOutputs.Add(0, Out);
		bDefinedUsdShader = true;
	}
}
#endif // USE_USD_SDK
