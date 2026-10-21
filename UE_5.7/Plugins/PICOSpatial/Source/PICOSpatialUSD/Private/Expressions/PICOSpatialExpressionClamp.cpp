#include "PICOSpatialExpressionClamp.h"

#include "Materials/MaterialExpressionClamp.h"

#if USE_USD_SDK
namespace
{
	static bool ConnectClampInputWithConversion(
		FShaderGraphUsdConvertorPICO* Convertor,
		const FString& NodeName,
		UMaterialExpression* Owner,
		int32 ExpressionInputIndex,
		const pxr::UsdShadeInput& TargetInput)
	{
		if (!Convertor || !Owner || !TargetInput)
		{
			return false;
		}

		FExpressionInput* Input = Owner->GetInput(ExpressionInputIndex);
		if (!Input)
		{
			return false;
		}

		if (!Input->IsConnected() || !Input->Expression)
		{
			return true;
		}

		FExpressionBasePICO* ConnectedNode = Convertor->FindNode(Input->Expression);
		if (!ConnectedNode)
		{
			UE_LOG(LogPICOSpatialUSD, Warning, TEXT("%s[CONNECT][FAIL] %s input %d: missing upstream node"),
				*Convertor->GetLogPrefix(), *NodeName, ExpressionInputIndex);
			return false;
		}

		if (!ConnectedNode->ShaderOutputs.Contains(Input->OutputIndex))
		{
			UE_LOG(LogPICOSpatialUSD, Warning, TEXT("%s[CONNECT][FAIL] %s input %d: missing upstream output %d"),
				*Convertor->GetLogPrefix(), *NodeName, ExpressionInputIndex, Input->OutputIndex);
			return false;
		}

		pxr::UsdShadeOutput SourceOutput = ConnectedNode->ShaderOutputs[Input->OutputIndex];
		pxr::SdfValueTypeName OutputType = ConnectedNode->GetOutputType(Input->OutputIndex);
		pxr::SdfValueTypeName InputType = TargetInput.GetTypeName();

		if (OutputType && InputType && OutputType != InputType)
		{
			pxr::UsdShadeShader ConvertShader = Convertor->CreateConvertOrSwizzleShader(OutputType, InputType);
			if (!ConvertShader)
			{
				UE_LOG(LogPICOSpatialUSD, Warning, TEXT("%s[CONNECT][FAIL] %s input %d: failed to create type conversion shader"),
					*Convertor->GetLogPrefix(), *NodeName, ExpressionInputIndex);
				return false;
			}

			pxr::UsdShadeInput ConvertInput = ConvertShader.GetInput(pxr::TfToken("in"));
			pxr::UsdShadeOutput ConvertOutput = ConvertShader.GetOutput(pxr::TfToken("out"));
			if (!ConvertInput || !ConvertOutput)
			{
				UE_LOG(LogPICOSpatialUSD, Warning, TEXT("%s[CONNECT][FAIL] %s input %d: missing conversion node pins"),
					*Convertor->GetLogPrefix(), *NodeName, ExpressionInputIndex);
				return false;
			}

			if (!ConvertInput.ConnectToSource(SourceOutput))
			{
				UE_LOG(LogPICOSpatialUSD, Warning, TEXT("%s[CONNECT][FAIL] %s input %d: failed to connect upstream to conversion node"),
					*Convertor->GetLogPrefix(), *NodeName, ExpressionInputIndex);
				return false;
			}

			return TargetInput.ConnectToSource(ConvertOutput);
		}

		return TargetInput.ConnectToSource(SourceOutput);
	}

	static FString GetClampNodeDefPrefix(EClampMode ClampMode)
	{
		switch (ClampMode)
		{
		case CMODE_ClampMin:
			return TEXT("ND_max");
		case CMODE_ClampMax:
			return TEXT("ND_min");
		case CMODE_Clamp:
		default:
			return TEXT("ND_clamp");
		}
	}
}

void FExpressionClampPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (UMaterialExpressionClamp* Clamp = Cast<UMaterialExpressionClamp>(Owner))
	{
		const FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
		if (ShaderPath.IsEmpty())
		{
			return;
		}

		const pxr::SdfValueTypeName T = InferredUsdType ? InferredUsdType : pxr::SdfValueTypeNames->Color4f;
		const FString Suffix = FShaderGraphUsdConvertorPICO::GetMxTypeSuffix(T);
		const bool bIsColor = FShaderGraphUsdConvertorPICO::IsColorUsdType(T);
		const FString NodeDefPrefix = GetClampNodeDefPrefix(Clamp->ClampMode);

		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
		UsdShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s_%s"), *NodeDefPrefix, *Suffix))));

		pxr::UsdShadeInput In = UsdShader.CreateInput(pxr::TfToken("in"), T);
		FShaderGraphUsdConvertorPICO::SetDefaultInputValue(In, 0.0f, T);
		if (bIsColor)
		{
			In.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
		}
		ShaderInputs.Add(0, In);

		if (Clamp->ClampMode != CMODE_ClampMax)
		{
			pxr::UsdShadeInput Low = UsdShader.CreateInput(
				pxr::TfToken(Clamp->ClampMode == CMODE_Clamp ? "low" : "in2"), T);
			FShaderGraphUsdConvertorPICO::SetDefaultInputValue(Low, Clamp->MinDefault, T);
			if (bIsColor)
			{
				Low.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
			}
			ShaderInputs.Add(1, Low);
		}

		if (Clamp->ClampMode != CMODE_ClampMin)
		{
			pxr::UsdShadeInput High = UsdShader.CreateInput(
				pxr::TfToken(Clamp->ClampMode == CMODE_Clamp ? "high" : "in2"), T);
			FShaderGraphUsdConvertorPICO::SetDefaultInputValue(High, Clamp->MaxDefault, T);
			if (bIsColor)
			{
				High.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
			}
			ShaderInputs.Add(2, High);
		}

		pxr::UsdShadeOutput Out = UsdShader.CreateOutput(pxr::TfToken("out"), T);
		ShaderOutputs.Add(0, Out);

		bDefinedUsdShader = true;
	}
}

bool FExpressionClampPICO::ConnectInput()
{
	bool bSuccess = true;
	if (ShaderInputs.Contains(0))
	{
		bSuccess = ConnectClampInputWithConversion(Convertor, NodeName, Owner, 0, ShaderInputs[0]) && bSuccess;
	}
	if (ShaderInputs.Contains(1))
	{
		bSuccess = ConnectClampInputWithConversion(Convertor, NodeName, Owner, 1, ShaderInputs[1]) && bSuccess;
	}
	if (ShaderInputs.Contains(2))
	{
		bSuccess = ConnectClampInputWithConversion(Convertor, NodeName, Owner, 2, ShaderInputs[2]) && bSuccess;
	}
	return bSuccess;
}

#endif // USE_USD_SDK
