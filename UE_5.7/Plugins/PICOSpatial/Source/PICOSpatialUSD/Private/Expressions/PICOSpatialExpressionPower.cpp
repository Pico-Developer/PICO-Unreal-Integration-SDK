#include "PICOSpatialExpressionPower.h"

#if USE_USD_SDK

void FExpressionPowerPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (UMaterialExpressionPower* Power = Cast<UMaterialExpressionPower>(Owner))
	{
		FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
		if (ShaderPath.IsEmpty())
		{
			return;
		}
		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));

		const pxr::SdfValueTypeName T = InferredUsdType ? InferredUsdType : pxr::SdfValueTypeNames->Color4f;
		const FString Suffix = FShaderGraphUsdConvertorPICO::GetMxTypeSuffix(T);
		const bool bIsColor = FShaderGraphUsdConvertorPICO::IsColorUsdType(T);

		UsdShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*FString::Printf(TEXT("ND_power_%s"), *Suffix))));

		pxr::UsdShadeInput in1 = UsdShader.CreateInput(pxr::TfToken("in1"), T);
		FShaderGraphUsdConvertorPICO::SetDefaultInputValue(in1, 0.0f, T);
		if (bIsColor) in1.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
		ShaderInputs.Add(0, in1);

		pxr::UsdShadeInput in2 = UsdShader.CreateInput(pxr::TfToken("in2"), T);
		FShaderGraphUsdConvertorPICO::SetDefaultInputValue(in2, Power->ConstExponent, T);
		if (bIsColor) in2.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
		ShaderInputs.Add(1, in2);

		pxr::UsdShadeOutput out = UsdShader.CreateOutput(pxr::TfToken("out"), T);
		ShaderOutputs.Add(0, out);

		bDefinedUsdShader = true;
	}
}

bool FExpressionPowerPICO::ConnectInput()
{
	bool bSuccess = true;
	FExpressionInput* Input = Owner->GetInput(0);
	if (!Input)
	{
		return false;
	}
	if (Input->IsConnected() && Input->Expression)
	{
		bSuccess = false;
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
	}

	Input = nullptr;
	Input = Owner->GetInput(1);
	if (!Input)
	{
		return false;
	}
	if (Input->IsConnected() && Input->Expression)
	{
		bSuccess = false;
		if (FExpressionBasePICO* ConnectedNode = Convertor->FindNode(Input->Expression))
		{
			if (ConnectedNode->ShaderOutputs.Contains(Input->OutputIndex) && ShaderInputs.Contains(1))
			{
				pxr::SdfValueTypeName OutputType = ConnectedNode->GetOutputType(Input->OutputIndex);
				pxr::SdfValueTypeName InputType = GetInputType(1);
				pxr::UsdShadeInput TargetInput = ShaderInputs[1];

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
					TEXT("Failed to connect %s input 1: missing source output %d or target input"),
					*NodeName, Input->OutputIndex);
			}
		}
	}
	return bSuccess;
}

#endif // USE_USD_SDK
