#include "PICOSpatialExpressionTime.h"

#include "Materials/MaterialExpressionTime.h"

#if USE_USD_SDK

void FExpressionTimePICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (UMaterialExpressionTime* Time = Cast<UMaterialExpressionTime>(Owner))
	{
		FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
		if (ShaderPath.IsEmpty())
		{
			return;
		}

		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
		UsdShader.CreateIdAttr().Set(pxr::TfToken("ND_time_float"));

		pxr::UsdShadeInput Fps = UsdShader.CreateInput(pxr::TfToken("fps"), pxr::SdfValueTypeNames->Float);
		Fps.Set(1.0f);
		ShaderInputs.Add(0, Fps);

		pxr::UsdShadeOutput Out = UsdShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float);

		if (Time->bOverride_Period && Time->Period > 0.0f)
		{
			FString WrapPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName() + TEXT("_PeriodWrap"));
			if (WrapPath.IsEmpty())
			{
				return;
			}

			pxr::UsdShadeShader WrapShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*WrapPath)));
			WrapShader.CreateIdAttr().Set(pxr::TfToken("ND_modulo_float"));

			pxr::UsdShadeInput In1 = WrapShader.CreateInput(pxr::TfToken("in1"), pxr::SdfValueTypeNames->Float);
			In1.Set(0.0f);

			pxr::UsdShadeInput In2 = WrapShader.CreateInput(pxr::TfToken("in2"), pxr::SdfValueTypeNames->Float);
			In2.Set(Time->Period);

			pxr::UsdShadeOutput WrapOut = WrapShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float);

			In1.ConnectToSource(Out);
			ShaderOutputs.Add(0, WrapOut);
		}
		else
		{
			ShaderOutputs.Add(0, Out);
		}

		bDefinedUsdShader = true;
	}
}

#endif // USE_USD_SDK

