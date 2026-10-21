#include "PICOSpatialExpressionNormalize.h"

#include "Materials/MaterialExpressionNormalize.h"

#if USE_USD_SDK
void FExpressionNormalizePICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (Cast<UMaterialExpressionNormalize>(Owner))
	{
		FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
		if (ShaderPath.IsEmpty())
		{
			return;
		}

		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
		// For normalize, color types must map to their vector equivalents because
		// ND_normalize_* only exists for vector2/vector3/vector4 in MaterialX.
		const pxr::SdfValueTypeName RawType = InferredUsdType ? InferredUsdType : pxr::SdfValueTypeNames->Float4;
		const pxr::SdfValueTypeName T = FShaderGraphUsdConvertorPICO::ToVectorEquivalent(RawType);
		const FString Suffix = FShaderGraphUsdConvertorPICO::GetMxTypeSuffix(T);

		UsdShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*FString::Printf(TEXT("ND_normalize_%s"), *Suffix))));

		pxr::UsdShadeInput In = UsdShader.CreateInput(pxr::TfToken("in"), T);
		FShaderGraphUsdConvertorPICO::SetDefaultInputValue(In, 0.0f, T);
		ShaderInputs.Add(0, In);

		pxr::UsdShadeOutput Out = UsdShader.CreateOutput(pxr::TfToken("out"), T);
		ShaderOutputs.Add(0, Out);

		bDefinedUsdShader = true;
	}
}

bool FExpressionNormalizePICO::ConnectInput()
{
	FExpressionInput* Input = Owner->GetInput(0);
	if (!Input)
	{
		return false;
	}

	if (!Input->IsConnected() || !Input->Expression)
	{
		return true;
	}

	bool bSuccess = false;
	if (FExpressionBasePICO* ConnectedNode = Convertor->FindNode(Input->Expression))
	{
		if (ConnectedNode->ShaderOutputs.Contains(Input->OutputIndex) && ShaderInputs.Contains(0))
		{
			pxr::SdfValueTypeName OutputType = ConnectedNode->GetOutputType(Input->OutputIndex);
			pxr::SdfValueTypeName InputType = GetInputType(0);
			pxr::UsdShadeInput TargetInput = ShaderInputs[0];

			if (OutputType && InputType && OutputType != InputType)
			{
				pxr::UsdShadeShader ConvertShader = Convertor->CreateConvertOrSwizzleShader(OutputType, InputType);
				if (ConvertShader)
				{
					pxr::UsdShadeInput ConvertInput = ConvertShader.GetInput(pxr::TfToken("in"));
					pxr::UsdShadeOutput ConvertOutput = ConvertShader.GetOutput(pxr::TfToken("out"));

					if (ConvertInput && ConvertOutput)
					{
						if (ConvertInput.ConnectToSource(ConnectedNode->ShaderOutputs[Input->OutputIndex]))
						{
							bSuccess = TargetInput.ConnectToSource(ConvertOutput);
						}
					}
				}
			}
			else
			{
				bSuccess = TargetInput.ConnectToSource(ConnectedNode->ShaderOutputs[Input->OutputIndex]);
			}
		}
		else
		{
			UE_LOG(LogPICOSpatialUSD, Warning,
				TEXT("Failed to connect %s input 0: missing source output %d or target input"),
				*NodeName, Input->OutputIndex);
		}
	}

	return bSuccess;
}
#endif // USE_USD_SDK
