#include "PICOSpatialExpressionMFPI.h"

#include "Materials/MaterialExpressionMaterialFunctionCall.h"

#if USE_USD_SDK
namespace
{
	static bool ConnectToInputWithConversion_MFPI(
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
			return true;
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
}

void FExpressionMFPIPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	UMaterialExpressionMaterialFunctionCall* FunctionCall = Cast<UMaterialExpressionMaterialFunctionCall>(Owner);
	if (!FunctionCall)
	{
		return;
	}

	const UMaterialFunctionInterface* Function = FunctionCall->MaterialFunction;
	const FString FunctionNameNoSpace = Function ? Function->GetName().Replace(TEXT(" "), TEXT("")) : Owner->GetName();
	const FString GraphPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/MF_") + FunctionNameNoSpace + TEXT("_nodegraph"));
	if (GraphPath.IsEmpty())
	{
		return;
	}

	NodeGraph = pxr::UsdShadeNodeGraph::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*GraphPath)));

	pxr::UsdShadeInput In = NodeGraph.CreateInput(pxr::TfToken("in"), pxr::SdfValueTypeNames->Float);
	In.Set(1.0f);

	pxr::UsdShadeOutput Out = NodeGraph.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float);

	const FString PiPath = GraphPath + TEXT("/PI");
	pxr::UsdShadeShader PiShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*PiPath)));
	PiShader.CreateIdAttr().Set(pxr::TfToken("ND_pi_float"));
	pxr::UsdShadeOutput PiOut = PiShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float);

	const FString MulPath = GraphPath + TEXT("/Multiply");
	pxr::UsdShadeShader MulShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*MulPath)));
	MulShader.CreateIdAttr().Set(pxr::TfToken("ND_multiply_float"));
	pxr::UsdShadeInput MulIn1 = MulShader.CreateInput(pxr::TfToken("in1"), pxr::SdfValueTypeNames->Float);
	MulIn1.Set(0.0f);
	pxr::UsdShadeInput MulIn2 = MulShader.CreateInput(pxr::TfToken("in2"), pxr::SdfValueTypeNames->Float);
	MulIn2.Set(1.0f);
	pxr::UsdShadeOutput MulOut = MulShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float);

	if (!MulIn1.ConnectToSource(In))
	{
		return;
	}

	if (!MulIn2.ConnectToSource(PiOut))
	{
		return;
	}

	if (!Out.ConnectToSource(MulOut))
	{
		return;
	}

	UsdShader = MulShader;
	ShaderInputs.Add(0, In);
	ShaderOutputs.Add(0, Out);

	bDefinedUsdShader = true;
}

bool FExpressionMFPIPICO::ConnectInput()
{
	if (!ShaderInputs.Contains(0))
	{
		return false;
	}

	return ConnectToInputWithConversion_MFPI(Convertor, NodeName, Owner, 0, ShaderInputs[0]);
}

#endif // USE_USD_SDK
