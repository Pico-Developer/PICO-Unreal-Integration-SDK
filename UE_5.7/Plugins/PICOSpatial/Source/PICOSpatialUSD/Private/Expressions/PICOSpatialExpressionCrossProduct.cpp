#include "PICOSpatialExpressionCrossProduct.h"

#include "Materials/MaterialExpressionCrossProduct.h"

#if USE_USD_SDK
namespace
{
	static bool ConnectCrossProductInputWithConversion(
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
}

void FExpressionCrossProductPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (Cast<UMaterialExpressionCrossProduct>(Owner))
	{
		const FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
		if (ShaderPath.IsEmpty())
		{
			return;
		}

		const pxr::SdfValueTypeName T = pxr::SdfValueTypeNames->Float3;

		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
		UsdShader.CreateIdAttr().Set(pxr::TfToken("ND_crossproduct_vector3"));

		pxr::UsdShadeInput In1 = UsdShader.CreateInput(pxr::TfToken("in1"), T);
		FShaderGraphUsdConvertorPICO::SetDefaultInputValue(In1, 0.0f, T);
		ShaderInputs.Add(0, In1);

		pxr::UsdShadeInput In2 = UsdShader.CreateInput(pxr::TfToken("in2"), T);
		FShaderGraphUsdConvertorPICO::SetDefaultInputValue(In2, 0.0f, T);
		ShaderInputs.Add(1, In2);

		pxr::UsdShadeOutput Out = UsdShader.CreateOutput(pxr::TfToken("out"), T);
		ShaderOutputs.Add(0, Out);

		bDefinedUsdShader = true;
	}
}

bool FExpressionCrossProductPICO::ConnectInput()
{
	bool bSuccess = true;
	if (ShaderInputs.Contains(0))
	{
		bSuccess = ConnectCrossProductInputWithConversion(Convertor, NodeName, Owner, 0, ShaderInputs[0]) && bSuccess;
	}
	if (ShaderInputs.Contains(1))
	{
		bSuccess = ConnectCrossProductInputWithConversion(Convertor, NodeName, Owner, 1, ShaderInputs[1]) && bSuccess;
	}
	return bSuccess;
}

#endif // USE_USD_SDK
