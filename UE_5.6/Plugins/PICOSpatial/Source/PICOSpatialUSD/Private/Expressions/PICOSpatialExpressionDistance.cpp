#include "PICOSpatialExpressionDistance.h"

#include "Materials/MaterialExpressionDistance.h"

#if USE_USD_SDK
namespace
{
	static bool ConnectToExpectedType_Distance(
		FShaderGraphUsdConvertorPICO* Convertor,
		const FString& NodeName,
		UMaterialExpression* Owner,
		int32 ExpressionInputIndex,
		const pxr::UsdShadeInput& TargetInput,
		const pxr::SdfValueTypeName& ExpectedType)
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

		if (OutputType && ExpectedType && OutputType != ExpectedType)
		{
			pxr::UsdShadeShader ConvertShader = Convertor->CreateConvertOrSwizzleShader(OutputType, ExpectedType);
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
}

void FExpressionDistancePICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (!Cast<UMaterialExpressionDistance>(Owner))
	{
		return;
	}

	const pxr::SdfValueTypeName T = InferredUsdType ? InferredUsdType : pxr::SdfValueTypeNames->Float;

	if (T == pxr::SdfValueTypeNames->Float)
	{
		const FString GraphPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName() + "_nodegraph");
		if (GraphPath.IsEmpty())
		{
			return;
		}

		NodeGraph = pxr::UsdShadeNodeGraph::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*GraphPath)));

		pxr::UsdShadeInput In1Input = NodeGraph.CreateInput(pxr::TfToken("in1"), pxr::SdfValueTypeNames->Float);
		In1Input.Set(0.0f);
		pxr::UsdShadeInput In2Input = NodeGraph.CreateInput(pxr::TfToken("in2"), pxr::SdfValueTypeNames->Float);
		In2Input.Set(0.0f);
		pxr::UsdShadeOutput OutOutput = NodeGraph.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float);

		ShaderInputs.Add(0, In1Input);
		ShaderInputs.Add(1, In2Input);
		ShaderOutputs.Add(0, OutOutput);

		pxr::UsdShadeShader SubtractShader = pxr::UsdShadeShader::Define(
			Stage, pxr::SdfPath(TCHAR_TO_UTF8(*(GraphPath + "/Subtract"))));
		SubtractShader.CreateIdAttr().Set(pxr::TfToken("ND_subtract_float"));
		SubtractShader.CreateInput(pxr::TfToken("in1"), pxr::SdfValueTypeNames->Float).Set(0.0f);
		SubtractShader.CreateInput(pxr::TfToken("in2"), pxr::SdfValueTypeNames->Float).Set(0.0f);
		SubtractShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float);

		pxr::UsdShadeShader AbsShader = pxr::UsdShadeShader::Define(
			Stage, pxr::SdfPath(TCHAR_TO_UTF8(*(GraphPath + "/Abs"))));
		AbsShader.CreateIdAttr().Set(pxr::TfToken("ND_absval_float"));
		AbsShader.CreateInput(pxr::TfToken("in"), pxr::SdfValueTypeNames->Float).Set(0.0f);
		AbsShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float);

		if (!SubtractShader.GetInput(pxr::TfToken("in1")).ConnectToSource(In1Input) ||
			!SubtractShader.GetInput(pxr::TfToken("in2")).ConnectToSource(In2Input) ||
			!AbsShader.GetInput(pxr::TfToken("in")).ConnectToSource(SubtractShader.GetOutput(pxr::TfToken("out"))) ||
			!OutOutput.ConnectToSource(AbsShader.GetOutput(pxr::TfToken("out"))))
		{
			return;
		}

		UsdShader = AbsShader;
		bDefinedUsdShader = true;
		return;
	}

	const FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
	if (ShaderPath.IsEmpty())
	{
		return;
	}

	const FString Suffix = FShaderGraphUsdConvertorPICO::GetMxTypeSuffix(T);

	UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
	UsdShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*FString::Printf(TEXT("ND_distance_%s"), *Suffix))));

	pxr::UsdShadeInput In1 = UsdShader.CreateInput(pxr::TfToken("in1"), T);
	FShaderGraphUsdConvertorPICO::SetDefaultInputValue(In1, 0.0f, T);
	ShaderInputs.Add(0, In1);

	pxr::UsdShadeInput In2 = UsdShader.CreateInput(pxr::TfToken("in2"), T);
	FShaderGraphUsdConvertorPICO::SetDefaultInputValue(In2, 0.0f, T);
	ShaderInputs.Add(1, In2);

	pxr::UsdShadeOutput Out = UsdShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float);
	ShaderOutputs.Add(0, Out);

	bDefinedUsdShader = true;
}

bool FExpressionDistancePICO::ConnectInput()
{
	const pxr::SdfValueTypeName ExpectedType =
		(InferredUsdType && InferredUsdType != pxr::SdfValueTypeNames->Float)
			? InferredUsdType
			: pxr::SdfValueTypeNames->Float;

	bool bSuccess = true;
	if (ShaderInputs.Contains(0))
	{
		bSuccess = ConnectToExpectedType_Distance(Convertor, NodeName, Owner, 0, ShaderInputs[0], ExpectedType) && bSuccess;
	}
	if (ShaderInputs.Contains(1))
	{
		bSuccess = ConnectToExpectedType_Distance(Convertor, NodeName, Owner, 1, ShaderInputs[1], ExpectedType) && bSuccess;
	}
	return bSuccess;
}

#endif // USE_USD_SDK
