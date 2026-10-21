#include "PICOSpatialExpressionAdd.h"

#if USE_USD_SDK

void FExpressionAddPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (UMaterialExpressionAdd* Add = Cast<UMaterialExpressionAdd>(Owner))
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

		UsdShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*FString::Printf(TEXT("ND_add_%s"), *Suffix))));

		pxr::UsdShadeInput in1 = UsdShader.CreateInput(pxr::TfToken("in1"), T);
		FShaderGraphUsdConvertorPICO::SetDefaultInputValue(in1, Add->ConstA, T);
		if (bIsColor) in1.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
		ShaderInputs.Add(0, in1);

		pxr::UsdShadeInput in2 = UsdShader.CreateInput(pxr::TfToken("in2"), T);
		FShaderGraphUsdConvertorPICO::SetDefaultInputValue(in2, Add->ConstB, T);
		if (bIsColor) in2.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
		ShaderInputs.Add(1, in2);

		pxr::UsdShadeOutput out = UsdShader.CreateOutput(pxr::TfToken("out"), T);
		ShaderOutputs.Add(0, out);

		bDefinedUsdShader = true;
	}
}

bool FExpressionAddPICO::ConnectInput()
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
				// Get output type from connected node and input type from this node
				pxr::SdfValueTypeName OutputType = ConnectedNode->GetOutputType(Input->OutputIndex);
				pxr::SdfValueTypeName InputType = GetInputType(0);
				pxr::UsdShadeInput TargetInput = ShaderInputs[0];

				// Check if type conversion is needed
				if (OutputType && InputType && OutputType != InputType)
				{
					// Create conversion shader
					pxr::UsdShadeShader ConvertShader = Convertor->CreateConvertOrSwizzleShader(OutputType, InputType);

					if (ConvertShader)
					{
						pxr::UsdShadeInput ConvertInput = ConvertShader.GetInput(pxr::TfToken("in"));
						pxr::UsdShadeOutput ConvertOutput = ConvertShader.GetOutput(pxr::TfToken("out"));

						if (ConvertInput && ConvertOutput)
						{
							// Connect source node -> convert node
							if (ConvertInput.ConnectToSource(ConnectedNode->ShaderOutputs[Input->OutputIndex]))
							{
								// Connect convert node -> this node's input
								bSuccess = TargetInput.ConnectToSource(ConvertOutput);

								if (bSuccess)
								{
									UE_LOG(LogPICOSpatialUSD, Verbose,
										TEXT("%s[CONNECT] %s -> %s input 0 (convert %s -> %s)"),
										Convertor ? *Convertor->GetLogPrefix() : TEXT("[USD_MTLX][#0000][MAT=<null>]"),
										*ConnectedNode->NodeName, *NodeName,
										UTF8_TO_TCHAR(OutputType.GetAsToken().GetString().c_str()),
										UTF8_TO_TCHAR(InputType.GetAsToken().GetString().c_str()));
								}
								else
								{
									UE_LOG(LogPICOSpatialUSD, Warning,
										TEXT("Failed to connect conversion node output to %s input 0"),
										*NodeName);
								}
							}
							else
							{
								UE_LOG(LogPICOSpatialUSD, Warning,
									TEXT("Failed to connect %s to conversion node input for %s input 0"),
									*ConnectedNode->NodeName, *NodeName);
							}
						}
						else
						{
							UE_LOG(LogPICOSpatialUSD, Warning,
								TEXT("Failed to get conversion node inputs/outputs for %s input 0"),
								*NodeName);
						}
					}
					else
					{
						UE_LOG(LogPICOSpatialUSD, Warning,
							TEXT("Failed to create type conversion shader for %s input 0"),
							*NodeName);
					}
				}
				else
				{
					// Direct connection without conversion
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
				// Get output type from connected node and input type from this node
				pxr::SdfValueTypeName OutputType = ConnectedNode->GetOutputType(Input->OutputIndex);
				pxr::SdfValueTypeName InputType = GetInputType(1);
				pxr::UsdShadeInput TargetInput = ShaderInputs[1];

				// Check if type conversion is needed
				if (OutputType && InputType && OutputType != InputType)
				{
					// Create conversion shader
					pxr::UsdShadeShader ConvertShader = Convertor->CreateConvertOrSwizzleShader(OutputType, InputType);

					if (ConvertShader)
					{
						pxr::UsdShadeInput ConvertInput = ConvertShader.GetInput(pxr::TfToken("in"));
						pxr::UsdShadeOutput ConvertOutput = ConvertShader.GetOutput(pxr::TfToken("out"));

						if (ConvertInput && ConvertOutput)
						{
							// Connect source node -> convert node
							if (ConvertInput.ConnectToSource(ConnectedNode->ShaderOutputs[Input->OutputIndex]))
							{
								// Connect convert node -> this node's input
								bSuccess = TargetInput.ConnectToSource(ConvertOutput);

								if (bSuccess)
								{
									UE_LOG(LogPICOSpatialUSD, Verbose,
										TEXT("%s[CONNECT] %s -> %s input 1 (convert %s -> %s)"),
										Convertor ? *Convertor->GetLogPrefix() : TEXT("[USD_MTLX][#0000][MAT=<null>]"),
										*ConnectedNode->NodeName, *NodeName,
										UTF8_TO_TCHAR(OutputType.GetAsToken().GetString().c_str()),
										UTF8_TO_TCHAR(InputType.GetAsToken().GetString().c_str()));
								}
								else
								{
									UE_LOG(LogPICOSpatialUSD, Warning,
										TEXT("Failed to connect conversion node output to %s input 1"),
										*NodeName);
								}
							}
							else
							{
								UE_LOG(LogPICOSpatialUSD, Warning,
									TEXT("Failed to connect %s to conversion node input for %s input 1"),
									*ConnectedNode->NodeName, *NodeName);
							}
						}
						else
						{
							UE_LOG(LogPICOSpatialUSD, Warning,
								TEXT("Failed to get conversion node inputs/outputs for %s input 1"),
								*NodeName);
						}
					}
					else
					{
						UE_LOG(LogPICOSpatialUSD, Warning,
							TEXT("Failed to create type conversion shader for %s input 1"),
							*NodeName);
					}
				}
				else
				{
					// Direct connection without conversion
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


