#include "PICOSpatialExpressionTrigonometric.h"

#include "Materials/MaterialExpressionArccosine.h"
#include "Materials/MaterialExpressionArccosineFast.h"
#include "Materials/MaterialExpressionArcsine.h"
#include "Materials/MaterialExpressionArcsineFast.h"
#include "Materials/MaterialExpressionArctangent2.h"
#include "Materials/MaterialExpressionArctangent2Fast.h"
#include "Materials/MaterialExpressionCosine.h"
#include "Materials/MaterialExpressionSine.h"
#include "Materials/MaterialExpressionTangent.h"

#if USE_USD_SDK
namespace
{
	static bool ConnectToInputWithConversion_Trig(
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

	// Build the zero-default value for a given numeric USD type.
	static pxr::VtValue MakeZeroValue(const pxr::SdfValueTypeName& T)
	{
		if (T == pxr::SdfValueTypeNames->Float2) return pxr::VtValue(pxr::GfVec2f(0.f, 0.f));
		if (T == pxr::SdfValueTypeNames->Float3) return pxr::VtValue(pxr::GfVec3f(0.f, 0.f, 0.f));
		if (T == pxr::SdfValueTypeNames->Float)  return pxr::VtValue(0.f);
		return pxr::VtValue(pxr::GfVec4f(0.f, 0.f, 0.f, 0.f)); // Float4 / fallback
	}

	static bool CreateUnaryTrigNodeGraph(
		pxr::UsdStageRefPtr Stage,
		FShaderGraphUsdConvertorPICO* Convertor,
		UMaterialExpression* Owner,
		const FString& TrigId,
		bool bUsePeriodScale,
		float Period,
		bool bPeriodNonPositiveUsesZeroInput,
		const pxr::SdfValueTypeName& IoType,     // inferred type for in/out
		pxr::UsdShadeNodeGraph& OutNodeGraph,
		pxr::UsdShadeInput& OutGraphInput,
		pxr::UsdShadeOutput& OutGraphOutput)
	{
		if (!Stage || !Convertor || !Owner)
		{
			return false;
		}

		const FString GraphPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName() + TEXT("_nodegraph"));
		if (GraphPath.IsEmpty())
		{
			return false;
		}

		OutNodeGraph = pxr::UsdShadeNodeGraph::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*GraphPath)));

		OutGraphInput = OutNodeGraph.CreateInput(pxr::TfToken("in"), IoType);
		OutGraphInput.GetAttr().Set(MakeZeroValue(IoType));

		OutGraphOutput = OutNodeGraph.CreateOutput(pxr::TfToken("out"), IoType);

		{
			const FString TrigPath = GraphPath + TEXT("/Trig");
			pxr::UsdShadeShader TrigShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*TrigPath)));
			TrigShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*TrigId)));

			pxr::UsdShadeInput TrigIn = TrigShader.CreateInput(pxr::TfToken("in"), IoType);
			TrigIn.GetAttr().Set(MakeZeroValue(IoType));

			pxr::UsdShadeOutput TrigOut = TrigShader.CreateOutput(pxr::TfToken("out"), IoType);

			if (bUsePeriodScale)
			{
				if (Period <= 0.0f && bPeriodNonPositiveUsesZeroInput)
				{
				}
				else if (Period > 0.0f)
				{
					const FString PiPath = GraphPath + TEXT("/PI");
					pxr::UsdShadeShader PiShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*PiPath)));
					PiShader.CreateIdAttr().Set(pxr::TfToken("ND_pi_float"));
					pxr::UsdShadeOutput PiOut = PiShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float);

					const FString TwoPath = GraphPath + TEXT("/Two");
					pxr::UsdShadeShader TwoShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*TwoPath)));
					TwoShader.CreateIdAttr().Set(pxr::TfToken("ND_constant_float"));
					pxr::UsdShadeInput TwoValue = TwoShader.CreateInput(pxr::TfToken("value"), pxr::SdfValueTypeNames->Float);
					TwoValue.Set(2.0f);
					pxr::UsdShadeOutput TwoOut = TwoShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float);

					const FString MulPi2Path = GraphPath + TEXT("/MulPi2");
					pxr::UsdShadeShader MulPi2Shader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*MulPi2Path)));
					MulPi2Shader.CreateIdAttr().Set(pxr::TfToken("ND_multiply_float"));
					pxr::UsdShadeInput MulPi2In1 = MulPi2Shader.CreateInput(pxr::TfToken("in1"), pxr::SdfValueTypeNames->Float);
					MulPi2In1.Set(0.0f);
					pxr::UsdShadeInput MulPi2In2 = MulPi2Shader.CreateInput(pxr::TfToken("in2"), pxr::SdfValueTypeNames->Float);
					MulPi2In2.Set(1.0f);
					pxr::UsdShadeOutput MulPi2Out = MulPi2Shader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float);

					if (!MulPi2In1.ConnectToSource(PiOut))
					{
						return false;
					}
					if (!MulPi2In2.ConnectToSource(TwoOut))
					{
						return false;
					}

					const FString DivPath = GraphPath + TEXT("/DivPeriod");
					pxr::UsdShadeShader DivShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*DivPath)));
					DivShader.CreateIdAttr().Set(pxr::TfToken("ND_divide_float"));
					pxr::UsdShadeInput DivIn1 = DivShader.CreateInput(pxr::TfToken("in1"), pxr::SdfValueTypeNames->Float);
					DivIn1.Set(0.0f);
					pxr::UsdShadeInput DivIn2 = DivShader.CreateInput(pxr::TfToken("in2"), pxr::SdfValueTypeNames->Float);
					DivIn2.Set(Period);
					pxr::UsdShadeOutput DivOut = DivShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Float);

					if (!DivIn1.ConnectToSource(MulPi2Out))
					{
						return false;
					}

				const FString IoSuffix = FShaderGraphUsdConvertorPICO::GetMxTypeSuffix(IoType);
				const FString ScaleNodeId = FString::Printf(TEXT("ND_multiply_%s"), *IoSuffix);
				const FString ScalePath = GraphPath + TEXT("/PeriodScale");
				pxr::UsdShadeShader ScaleShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ScalePath)));
				ScaleShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*ScaleNodeId)));
				pxr::UsdShadeInput ScaleIn1 = ScaleShader.CreateInput(pxr::TfToken("in1"), IoType);
				ScaleIn1.GetAttr().Set(MakeZeroValue(IoType));
				pxr::UsdShadeInput ScaleIn2 = ScaleShader.CreateInput(pxr::TfToken("in2"), IoType);
				ScaleIn2.GetAttr().Set(MakeZeroValue(IoType));
				pxr::UsdShadeOutput ScaleOut = ScaleShader.CreateOutput(pxr::TfToken("out"), IoType);

					if (!ScaleIn1.ConnectToSource(OutGraphInput))
					{
						return false;
					}
					if (IoType == pxr::SdfValueTypeNames->Float)
					{
						if (!ScaleIn2.ConnectToSource(DivOut))
						{
							return false;
						}
					}
					else
					{
						FString ConvertId;
						if (IoType == pxr::SdfValueTypeNames->Float2) ConvertId = TEXT("ND_convert_float_vector2");
						else if (IoType == pxr::SdfValueTypeNames->Float3) ConvertId = TEXT("ND_convert_float_vector3");
						else ConvertId = TEXT("ND_convert_float_vector4");

						const FString ConvertPath = GraphPath + TEXT("/PeriodScaleConvert");
						pxr::UsdShadeShader PeriodConvertShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ConvertPath)));
						PeriodConvertShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*ConvertId)));

						pxr::UsdShadeInput PeriodConvertIn = PeriodConvertShader.CreateInput(pxr::TfToken("in"), pxr::SdfValueTypeNames->Float);
						PeriodConvertIn.Set(0.0f);
						pxr::UsdShadeOutput PeriodConvertOut = PeriodConvertShader.CreateOutput(pxr::TfToken("out"), IoType);

						if (!PeriodConvertIn.ConnectToSource(DivOut))
						{
							return false;
						}
						if (!ScaleIn2.ConnectToSource(PeriodConvertOut))
						{
							return false;
						}
					}
					if (!TrigIn.ConnectToSource(ScaleOut))
					{
						return false;
					}
				}
				else
				{
					if (!TrigIn.ConnectToSource(OutGraphInput))
					{
						return false;
					}
				}
			}
			else
			{
				if (!TrigIn.ConnectToSource(OutGraphInput))
				{
					return false;
				}
			}

			if (!OutGraphOutput.ConnectToSource(TrigOut))
			{
				return false;
			}
		}

		return true;
	}

}

void FExpressionSinePICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (Cast<UMaterialExpressionSine>(Owner))
	{
		UMaterialExpressionSine* Sine = Cast<UMaterialExpressionSine>(Owner);
		if (!Sine)
		{
			return;
		}

		const pxr::SdfValueTypeName RawType = InferredUsdType ? InferredUsdType : pxr::SdfValueTypeNames->Float4;
		const pxr::SdfValueTypeName IoType = FShaderGraphUsdConvertorPICO::ToVectorEquivalent(RawType);
		const FString Suffix = FShaderGraphUsdConvertorPICO::GetMxTypeSuffix(IoType);
		const FString TrigNodeId = FString::Printf(TEXT("ND_sin_%s"), *Suffix);

		pxr::UsdShadeInput GraphIn;
		pxr::UsdShadeOutput GraphOut;
		if (!CreateUnaryTrigNodeGraph(Stage, Convertor, Owner, TrigNodeId, true, Sine->Period, false, IoType, NodeGraph, GraphIn, GraphOut))
		{
			return;
		}

		ShaderInputs.Add(0, GraphIn);
		ShaderOutputs.Add(0, GraphOut);

		bDefinedUsdShader = true;
	}
}

bool FExpressionSinePICO::ConnectInput()
{
	if (!ShaderInputs.Contains(0))
	{
		return false;
	}

	UMaterialExpressionSine* Sine = Cast<UMaterialExpressionSine>(Owner);
	if (!Sine)
	{
		return false;
	}

	if (Sine->Period <= 0.0f)
	{
		return true;
	}

	return ConnectToInputWithConversion_Trig(Convertor, NodeName, Owner, 0, ShaderInputs[0]);
}

void FExpressionCosinePICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (Cast<UMaterialExpressionCosine>(Owner))
	{
		UMaterialExpressionCosine* Cosine = Cast<UMaterialExpressionCosine>(Owner);
		if (!Cosine)
		{
			return;
		}

		const pxr::SdfValueTypeName RawType = InferredUsdType ? InferredUsdType : pxr::SdfValueTypeNames->Float4;
		const pxr::SdfValueTypeName IoType = FShaderGraphUsdConvertorPICO::ToVectorEquivalent(RawType);
		const FString Suffix = FShaderGraphUsdConvertorPICO::GetMxTypeSuffix(IoType);
		const FString TrigNodeId = FString::Printf(TEXT("ND_cos_%s"), *Suffix);

		pxr::UsdShadeInput GraphIn;
		pxr::UsdShadeOutput GraphOut;
		if (!CreateUnaryTrigNodeGraph(Stage, Convertor, Owner, TrigNodeId, true, Cosine->Period, true, IoType, NodeGraph, GraphIn, GraphOut))
		{
			return;
		}

		ShaderInputs.Add(0, GraphIn);
		ShaderOutputs.Add(0, GraphOut);

		bDefinedUsdShader = true;
	}
}

bool FExpressionCosinePICO::ConnectInput()
{
	if (!ShaderInputs.Contains(0))
	{
		return false;
	}

	UMaterialExpressionCosine* Cosine = Cast<UMaterialExpressionCosine>(Owner);
	if (!Cosine)
	{
		return false;
	}

	if (Cosine->Period <= 0.0f)
	{
		return true;
	}

	return ConnectToInputWithConversion_Trig(Convertor, NodeName, Owner, 0, ShaderInputs[0]);
}

void FExpressionArcsinePICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (Cast<UMaterialExpressionArcsine>(Owner) || Cast<UMaterialExpressionArcsineFast>(Owner))
	{
		FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
		if (ShaderPath.IsEmpty())
		{
			return;
		}

		const pxr::SdfValueTypeName RawType = InferredUsdType ? InferredUsdType : pxr::SdfValueTypeNames->Float4;
		const pxr::SdfValueTypeName IoType = FShaderGraphUsdConvertorPICO::ToVectorEquivalent(RawType);
		const FString Suffix = FShaderGraphUsdConvertorPICO::GetMxTypeSuffix(IoType);

		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
		UsdShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*FString::Printf(TEXT("ND_asin_%s"), *Suffix))));

		pxr::UsdShadeInput In = UsdShader.CreateInput(pxr::TfToken("in"), IoType);
		In.GetAttr().Set(MakeZeroValue(IoType));
		ShaderInputs.Add(0, In);

		pxr::UsdShadeOutput Out = UsdShader.CreateOutput(pxr::TfToken("out"), IoType);
		ShaderOutputs.Add(0, Out);

		bDefinedUsdShader = true;
	}
}

bool FExpressionArcsinePICO::ConnectInput()
{
	if (!ShaderInputs.Contains(0))
	{
		return false;
	}

	return ConnectToInputWithConversion_Trig(Convertor, NodeName, Owner, 0, ShaderInputs[0]);
}

void FExpressionArccosinePICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (Cast<UMaterialExpressionArccosine>(Owner) || Cast<UMaterialExpressionArccosineFast>(Owner))
	{
		FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
		if (ShaderPath.IsEmpty())
		{
			return;
		}

		const pxr::SdfValueTypeName RawType = InferredUsdType ? InferredUsdType : pxr::SdfValueTypeNames->Float4;
		const pxr::SdfValueTypeName IoType = FShaderGraphUsdConvertorPICO::ToVectorEquivalent(RawType);
		const FString Suffix = FShaderGraphUsdConvertorPICO::GetMxTypeSuffix(IoType);

		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
		UsdShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*FString::Printf(TEXT("ND_acos_%s"), *Suffix))));

		pxr::UsdShadeInput In = UsdShader.CreateInput(pxr::TfToken("in"), IoType);
		In.GetAttr().Set(MakeZeroValue(IoType));
		ShaderInputs.Add(0, In);

		pxr::UsdShadeOutput Out = UsdShader.CreateOutput(pxr::TfToken("out"), IoType);
		ShaderOutputs.Add(0, Out);

		bDefinedUsdShader = true;
	}
}

bool FExpressionArccosinePICO::ConnectInput()
{
	if (!ShaderInputs.Contains(0))
	{
		return false;
	}

	return ConnectToInputWithConversion_Trig(Convertor, NodeName, Owner, 0, ShaderInputs[0]);
}

void FExpressionTangentPICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (Cast<UMaterialExpressionTangent>(Owner))
	{
		UMaterialExpressionTangent* Tangent = Cast<UMaterialExpressionTangent>(Owner);
		if (!Tangent)
		{
			return;
		}

		const pxr::SdfValueTypeName RawType = InferredUsdType ? InferredUsdType : pxr::SdfValueTypeNames->Float4;
		const pxr::SdfValueTypeName IoType = FShaderGraphUsdConvertorPICO::ToVectorEquivalent(RawType);
		const FString Suffix = FShaderGraphUsdConvertorPICO::GetMxTypeSuffix(IoType);
		const FString TrigNodeId = FString::Printf(TEXT("ND_tan_%s"), *Suffix);

		pxr::UsdShadeInput GraphIn;
		pxr::UsdShadeOutput GraphOut;
		if (!CreateUnaryTrigNodeGraph(Stage, Convertor, Owner, TrigNodeId, true, Tangent->Period, true, IoType, NodeGraph, GraphIn, GraphOut))
		{
			return;
		}

		ShaderInputs.Add(0, GraphIn);
		ShaderOutputs.Add(0, GraphOut);

		bDefinedUsdShader = true;
	}
}

bool FExpressionTangentPICO::ConnectInput()
{
	if (!ShaderInputs.Contains(0))
	{
		return false;
	}

	UMaterialExpressionTangent* Tangent = Cast<UMaterialExpressionTangent>(Owner);
	if (!Tangent)
	{
		return false;
	}

	if (Tangent->Period <= 0.0f)
	{
		return true;
	}

	return ConnectToInputWithConversion_Trig(Convertor, NodeName, Owner, 0, ShaderInputs[0]);
}

void FExpressionArctangent2PICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (Cast<UMaterialExpressionArctangent2>(Owner) || Cast<UMaterialExpressionArctangent2Fast>(Owner))
	{
		FString ShaderPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName());
		if (ShaderPath.IsEmpty())
		{
			return;
		}

		const pxr::SdfValueTypeName RawType = InferredUsdType ? InferredUsdType : pxr::SdfValueTypeNames->Float4;
		const pxr::SdfValueTypeName IoType = FShaderGraphUsdConvertorPICO::ToVectorEquivalent(RawType);
		const FString Suffix = FShaderGraphUsdConvertorPICO::GetMxTypeSuffix(IoType);

		UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ShaderPath)));
		UsdShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*FString::Printf(TEXT("ND_atan2_%s"), *Suffix))));

		const pxr::VtValue ZeroVal = MakeZeroValue(IoType);
		pxr::UsdShadeInput In1 = UsdShader.CreateInput(pxr::TfToken("in1"), IoType);
		In1.GetAttr().Set(ZeroVal);
		ShaderInputs.Add(0, In1);

		pxr::UsdShadeInput In2 = UsdShader.CreateInput(pxr::TfToken("in2"), IoType);
		In2.GetAttr().Set(ZeroVal);
		ShaderInputs.Add(1, In2);

		pxr::UsdShadeOutput Out = UsdShader.CreateOutput(pxr::TfToken("out"), IoType);
		ShaderOutputs.Add(0, Out);

		bDefinedUsdShader = true;
	}
}

bool FExpressionArctangent2PICO::ConnectInput()
{
	if (!ShaderInputs.Contains(0) || !ShaderInputs.Contains(1))
	{
		return false;
	}

	bool bAnyConnected = false;
	bool bSuccess = true;

	if (FExpressionInput* InputY = Owner->GetInput(0); InputY && InputY->IsConnected() && InputY->Expression)
	{
		bAnyConnected = true;
		bSuccess = ConnectToInputWithConversion_Trig(Convertor, NodeName, Owner, 0, ShaderInputs[0]) && bSuccess;
	}

	if (FExpressionInput* InputX = Owner->GetInput(1); InputX && InputX->IsConnected() && InputX->Expression)
	{
		bAnyConnected = true;
		bSuccess = ConnectToInputWithConversion_Trig(Convertor, NodeName, Owner, 1, ShaderInputs[1]) && bSuccess;
	}

	return bAnyConnected ? bSuccess : true;
}

#endif // USE_USD_SDK
