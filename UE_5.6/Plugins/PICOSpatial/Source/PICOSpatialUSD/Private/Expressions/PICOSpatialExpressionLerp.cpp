#include "PICOSpatialExpressionLerp.h"

#include "Materials/MaterialExpressionLinearInterpolate.h"

#if USE_USD_SDK

void FExpressionLerpPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (UMaterialExpressionLinearInterpolate* Lerp = Cast<UMaterialExpressionLinearInterpolate>(Owner))
	{
		const FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
		if (ShaderPath.IsEmpty())
		{
			return;
		}
		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));

		const pxr::SdfValueTypeName T = InferredUsdType ? InferredUsdType : pxr::SdfValueTypeNames->Color4f;
		const FString Suffix = FShaderGraphUsdConvertorPICO::GetMxTypeSuffix(T);
		const bool bIsColor = FShaderGraphUsdConvertorPICO::IsColorUsdType(T);

		UsdShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*FString::Printf(TEXT("ND_mix_%s"), *Suffix))));

		pxr::UsdShadeInput BgInput = UsdShader.CreateInput(pxr::TfToken("bg"), T);
		FShaderGraphUsdConvertorPICO::SetDefaultInputValue(BgInput, Lerp->ConstA, T);
		if (bIsColor) BgInput.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
		ShaderInputs.Add(0, BgInput);

		pxr::UsdShadeInput FgInput = UsdShader.CreateInput(pxr::TfToken("fg"), T);
		FShaderGraphUsdConvertorPICO::SetDefaultInputValue(FgInput, Lerp->ConstB, T);
		if (bIsColor) FgInput.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
		ShaderInputs.Add(1, FgInput);

		pxr::UsdShadeInput MixInput = UsdShader.CreateInput(pxr::TfToken("mix"), pxr::SdfValueTypeNames->Float);
		MixInput.Set(Lerp->ConstAlpha);
		ShaderInputs.Add(2, MixInput);

		pxr::UsdShadeOutput Out = UsdShader.CreateOutput(pxr::TfToken("out"), T);
		ShaderOutputs.Add(0, Out);

		bDefinedUsdShader = true;
	}
}

static bool ConnectExpressionInputWithConversion(
	FShaderGraphUsdConvertorPICO* Convertor,
	const FString& NodeName,
	const FExpressionInput* Input,
	int32 TargetInputIndex,
	TMap<int, pxr::UsdShadeInput>& TargetShaderInputs)
{
	if (!Input)
	{
		return false;
	}

	if (!Input->IsConnected() || !Input->Expression)
	{
		return true;
	}

	if (!TargetShaderInputs.Contains(TargetInputIndex))
	{
		UE_LOG(LogPICOSpatialUSD, Warning,
			TEXT("Failed to connect %s input %d: missing target input"),
			*NodeName, TargetInputIndex);
		return false;
	}

	FExpressionBasePICO* ConnectedNode = Convertor->FindNode(Input->Expression);
	if (!ConnectedNode || !ConnectedNode->bDefinedUsdShader)
	{
		UE_LOG(LogPICOSpatialUSD, Warning,
			TEXT("Failed to connect %s input %d: missing connected node"),
			*NodeName, TargetInputIndex);
		return false;
	}

	if (!ConnectedNode->ShaderOutputs.Contains(Input->OutputIndex))
	{
		UE_LOG(LogPICOSpatialUSD, Warning,
			TEXT("Failed to connect %s input %d: missing source output %d"),
			*NodeName, TargetInputIndex, Input->OutputIndex);
		return false;
	}

	pxr::UsdShadeInput TargetInput = TargetShaderInputs[TargetInputIndex];
	pxr::UsdShadeOutput SourceOutput = ConnectedNode->ShaderOutputs[Input->OutputIndex];

	pxr::SdfValueTypeName OutputType = ConnectedNode->GetOutputType(Input->OutputIndex);
	pxr::SdfValueTypeName InputType = TargetInput.GetTypeName();

	if (OutputType && InputType && OutputType != InputType)
	{
		pxr::UsdShadeShader ConvertShader = Convertor->CreateConvertOrSwizzleShader(OutputType, InputType);
		if (!ConvertShader)
		{
			return false;
		}

		pxr::UsdShadeInput ConvertInput = ConvertShader.GetInput(pxr::TfToken("in"));
		pxr::UsdShadeOutput ConvertOutput = ConvertShader.GetOutput(pxr::TfToken("out"));
		if (!ConvertInput || !ConvertOutput)
		{
			return false;
		}

		return ConvertInput.ConnectToSource(SourceOutput) && TargetInput.ConnectToSource(ConvertOutput);
	}

	return TargetInput.ConnectToSource(SourceOutput);
}

bool FExpressionLerpPICO::ConnectInput()
{
	if (UMaterialExpressionLinearInterpolate* Lerp = Cast<UMaterialExpressionLinearInterpolate>(Owner))
	{
		bool bSuccess = true;
		bSuccess &= ConnectExpressionInputWithConversion(Convertor, NodeName, &Lerp->A, 0, ShaderInputs);
		bSuccess &= ConnectExpressionInputWithConversion(Convertor, NodeName, &Lerp->B, 1, ShaderInputs);
		bSuccess &= ConnectExpressionInputWithConversion(Convertor, NodeName, &Lerp->Alpha, 2, ShaderInputs);
		return bSuccess;
	}
	return false;
}

#endif // USE_USD_SDK
