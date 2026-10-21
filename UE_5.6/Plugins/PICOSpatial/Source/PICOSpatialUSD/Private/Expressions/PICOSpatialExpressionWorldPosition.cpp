#include "PICOSpatialExpressionWorldPosition.h"

#include "Materials/MaterialExpressionWorldPosition.h"

#if USE_USD_SDK
void FExpressionWorldPositionPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (UMaterialExpressionWorldPosition* WorldPosition = Cast<UMaterialExpressionWorldPosition>(Owner))
	{
		if (WorldPosition->WorldPositionShaderOffset != EWorldPositionIncludedOffsets::WPT_Default)
		{
			UE_LOG(LogPICOSpatialUSD, Warning, TEXT("%s[FAIL][WorldPosition] Unsupported WorldPositionShaderOffset=%d for node '%s'"),
				*Convertor->GetLogPrefix(), (int32)WorldPosition->WorldPositionShaderOffset.GetValue(), *NodeName);
			return;
		}

		FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/Position"));
		if (ShaderPath.IsEmpty())
		{
			return;
		}

		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
		UsdShader.CreateIdAttr().Set(pxr::TfToken("ND_position_vector3"));

		pxr::UsdShadeInput Space = UsdShader.CreateInput(pxr::TfToken("space"), pxr::SdfValueTypeNames->String);
		Space.Set(std::string("world"));

		pxr::UsdShadeOutput Out = UsdShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float3);
		ShaderOutputs.Add(0, Out);

		bDefinedUsdShader = true;
	}
}
#endif // USE_USD_SDK
