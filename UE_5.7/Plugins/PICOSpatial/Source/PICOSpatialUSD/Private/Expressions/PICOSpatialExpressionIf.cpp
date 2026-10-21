#include "PICOSpatialExpressionIf.h"

#if USE_USD_SDK

void FExpressionIfPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (UMaterialExpressionIf* IfExpression = Cast<UMaterialExpressionIf>(Owner))
	{
		const FString GraphPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName() + "_nodegraph");
		if (GraphPath.IsEmpty())
		{
			return;
		}

		NodeGraph = pxr::UsdShadeNodeGraph::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*GraphPath)));

		const pxr::SdfValueTypeName T = InferredUsdType ? InferredUsdType : pxr::SdfValueTypeNames->Color4f;
		const FString Suffix = FShaderGraphUsdConvertorPICO::GetMxTypeSuffix(T);
		const bool bIsColor = FShaderGraphUsdConvertorPICO::IsColorUsdType(T);

		pxr::UsdShadeInput AInput = NodeGraph.CreateInput(pxr::TfToken("A"), pxr::SdfValueTypeNames->Float);
		AInput.Set(0.0f);

		pxr::UsdShadeInput BInput = NodeGraph.CreateInput(pxr::TfToken("B"), pxr::SdfValueTypeNames->Float);
		BInput.Set(IfExpression->ConstB);

		pxr::UsdShadeInput GreaterValueInput = NodeGraph.CreateInput(pxr::TfToken("AGreaterThanB"), T);
		FShaderGraphUsdConvertorPICO::SetDefaultInputValue(GreaterValueInput, 0.0f, T);
		if (bIsColor) GreaterValueInput.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));

		pxr::UsdShadeInput EqualsValueInput = NodeGraph.CreateInput(pxr::TfToken("AEqualsB"), T);
		FShaderGraphUsdConvertorPICO::SetDefaultInputValue(EqualsValueInput, 0.0f, T);
		if (bIsColor) EqualsValueInput.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));

		pxr::UsdShadeInput LessValueInput = NodeGraph.CreateInput(pxr::TfToken("ALessThanB"), T);
		FShaderGraphUsdConvertorPICO::SetDefaultInputValue(LessValueInput, 0.0f, T);
		if (bIsColor) LessValueInput.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));

		pxr::UsdShadeOutput DefaultOutput = NodeGraph.CreateOutput(pxr::TfToken("Default"), T);

		ShaderInputs.Add(0, AInput);
		ShaderInputs.Add(1, BInput);
		ShaderInputs.Add(2, GreaterValueInput);
		ShaderInputs.Add(3, EqualsValueInput);
		ShaderInputs.Add(4, LessValueInput);

		{
			const FString ShaderPath = GraphPath + "/IfEqual";
			IfEqualShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
			IfEqualShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*FString::Printf(TEXT("ND_ifequal_%s"), *Suffix))));

			pxr::UsdShadeInput In1 = IfEqualShader.CreateInput(pxr::TfToken("in1"), T);
			FShaderGraphUsdConvertorPICO::SetDefaultInputValue(In1, 0.0f, T);
			if (bIsColor) In1.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));

			pxr::UsdShadeInput In2 = IfEqualShader.CreateInput(pxr::TfToken("in2"), T);
			FShaderGraphUsdConvertorPICO::SetDefaultInputValue(In2, 0.0f, T);
			if (bIsColor) In2.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));

			IfEqualShader.CreateInput(pxr::TfToken("value1"), pxr::SdfValueTypeNames->Float).Set(0.0f);
			IfEqualShader.CreateInput(pxr::TfToken("value2"), pxr::SdfValueTypeNames->Float).Set(0.0f);
			IfEqualShader.CreateOutput(pxr::TfToken("out"), T);
		}

		// Note: ND_ifgreater is NOT needed here. The three-way branch is implemented
		// as: IfGreaterOrEqual(A>=B: AGreaterThanB, A<B: ALessThanB) â†?fed into
		// IfEqual(A==B: AEqualsB, else: IfGreaterOrEqual.out). This correctly
		// reproduces UE's A>B / A==B / A<B semantics without a standalone IfGreater node.

		{
			const FString ShaderPath = GraphPath + "/IfGreaterOrEqual";
			IfGreaterOrEqualShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
			IfGreaterOrEqualShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*FString::Printf(TEXT("ND_ifgreatereq_%s"), *Suffix))));

			pxr::UsdShadeInput In1 = IfGreaterOrEqualShader.CreateInput(pxr::TfToken("in1"), T);
			FShaderGraphUsdConvertorPICO::SetDefaultInputValue(In1, 0.0f, T);
			if (bIsColor) In1.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));

			pxr::UsdShadeInput In2 = IfGreaterOrEqualShader.CreateInput(pxr::TfToken("in2"), T);
			FShaderGraphUsdConvertorPICO::SetDefaultInputValue(In2, 0.0f, T);
			if (bIsColor) In2.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));

			IfGreaterOrEqualShader.CreateInput(pxr::TfToken("value1"), pxr::SdfValueTypeNames->Float).Set(1.0f);
			IfGreaterOrEqualShader.CreateInput(pxr::TfToken("value2"), pxr::SdfValueTypeNames->Float).Set(0.0f);
			IfGreaterOrEqualShader.CreateOutput(pxr::TfToken("out"), T);
		}

		if (!IfGreaterOrEqualShader.GetInput(pxr::TfToken("value1")).ConnectToSource(AInput))
		{
			return;
		}
		if (!IfGreaterOrEqualShader.GetInput(pxr::TfToken("value2")).ConnectToSource(BInput))
		{
			return;
		}
		if (!IfGreaterOrEqualShader.GetInput(pxr::TfToken("in1")).ConnectToSource(GreaterValueInput))
		{
			return;
		}
		if (!IfGreaterOrEqualShader.GetInput(pxr::TfToken("in2")).ConnectToSource(LessValueInput))
		{
			return;
		}

		if (!IfEqualShader.GetInput(pxr::TfToken("value1")).ConnectToSource(AInput))
		{
			return;
		}
		if (!IfEqualShader.GetInput(pxr::TfToken("value2")).ConnectToSource(BInput))
		{
			return;
		}
		if (!IfEqualShader.GetInput(pxr::TfToken("in1")).ConnectToSource(EqualsValueInput))
		{
			return;
		}
		if (!IfEqualShader.GetInput(pxr::TfToken("in2")).ConnectToSource(IfGreaterOrEqualShader.GetOutput(pxr::TfToken("out"))))
		{
			return;
		}

		if (!DefaultOutput.ConnectToSource(IfEqualShader.GetOutput(pxr::TfToken("out"))))
		{
			return;
		}

		UsdShader = IfEqualShader;
		ShaderOutputs.Add(0, DefaultOutput);

		bDefinedUsdShader = true;
	}
}

bool FExpressionIfPICO::ConnectInput()
{
	bool bSuccess = true;

	UMaterialExpressionIf* IfExpression = Cast<UMaterialExpressionIf>(Owner);
	if (!IfExpression)
	{
		return bSuccess;
	}

	auto ConnectToExpectedType = [&](FExpressionInput* Input, const pxr::UsdShadeInput& Target, const pxr::SdfValueTypeName& ExpectedType) -> bool
	{
		if (!Target)
		{
			return false;
		}

		if (!Input || !Input->IsConnected() || !Input->Expression)
		{
			return true;
		}

		FExpressionBasePICO* ConnectedNode = Convertor->FindNode(Input->Expression);
		if (!ConnectedNode)
		{
			return false;
		}

		if (!ConnectedNode->ShaderOutputs.Contains(Input->OutputIndex))
		{
			return false;
		}

		pxr::UsdShadeOutput SourceOutput = ConnectedNode->ShaderOutputs[Input->OutputIndex];
		pxr::SdfValueTypeName OutputType = ConnectedNode->GetOutputType(Input->OutputIndex);

		if (OutputType && ExpectedType && OutputType != ExpectedType)
		{
			pxr::UsdShadeShader ConvertShader = Convertor->CreateConvertOrSwizzleShader(OutputType, ExpectedType);
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

			if (!ConvertInput.ConnectToSource(SourceOutput))
			{
				return false;
			}

			return Target.ConnectToSource(ConvertOutput);
		}

		return Target.ConnectToSource(SourceOutput);
	};

	pxr::UsdShadeInput AInput = NodeGraph.GetInput(pxr::TfToken("A"));
	pxr::UsdShadeInput BInput = NodeGraph.GetInput(pxr::TfToken("B"));
	pxr::UsdShadeInput GreaterValueInput = NodeGraph.GetInput(pxr::TfToken("AGreaterThanB"));
	pxr::UsdShadeInput LessValueInput = NodeGraph.GetInput(pxr::TfToken("ALessThanB"));
	pxr::UsdShadeInput EqualsValueInput = NodeGraph.GetInput(pxr::TfToken("AEqualsB"));

	if (IfExpression->A.IsConnected())
	{
		bSuccess = ConnectToExpectedType(&IfExpression->A, AInput, pxr::SdfValueTypeNames->Float) && bSuccess;
	}

	if (IfExpression->B.IsConnected())
	{
		bSuccess = ConnectToExpectedType(&IfExpression->B, BInput, pxr::SdfValueTypeNames->Float) && bSuccess;
	}
	else
	{
		BInput.Set(IfExpression->ConstB);
	}

	const pxr::SdfValueTypeName VT = InferredUsdType ? InferredUsdType : pxr::SdfValueTypeNames->Color4f;

	if (IfExpression->AGreaterThanB.IsConnected())
	{
		bSuccess = ConnectToExpectedType(&IfExpression->AGreaterThanB, GreaterValueInput, VT) && bSuccess;
	}

	if (IfExpression->ALessThanB.IsConnected())
	{
		bSuccess = ConnectToExpectedType(&IfExpression->ALessThanB, LessValueInput, VT) && bSuccess;
	}

	FExpressionInput* EqualsInput = IfExpression->AEqualsB.IsConnected() ? &IfExpression->AEqualsB : &IfExpression->AGreaterThanB;
	if (EqualsInput && EqualsInput->IsConnected())
	{
		bSuccess = ConnectToExpectedType(EqualsInput, EqualsValueInput, VT) && bSuccess;
	}

	return bSuccess;
}

#endif // USE_USD_SDK
