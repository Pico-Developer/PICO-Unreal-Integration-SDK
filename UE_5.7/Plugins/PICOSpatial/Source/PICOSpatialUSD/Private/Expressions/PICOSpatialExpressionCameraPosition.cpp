#include "PICOSpatialExpressionCameraPosition.h"

#include "Materials/MaterialExpressionCameraPositionWS.h"

#if USE_USD_SDK
void FExpressionCameraPositionPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (Cast<UMaterialExpressionCameraPositionWS>(Owner))
	{
		FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
		if (ShaderPath.IsEmpty())
		{
			return;
		}

		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
		UsdShader.CreateIdAttr().Set(pxr::TfToken("ND__cameraposition_vector3"));

		pxr::UsdShadeInput Space = UsdShader.CreateInput(pxr::TfToken("space"), pxr::SdfValueTypeNames->String);
		Space.Set(std::string("world"));

		pxr::UsdShadeOutput Out = UsdShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float3);

		ShaderOutputs.Add(0, Out);
		bDefinedUsdShader = true;
	}
}
#endif // USE_USD_SDK
