// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialShaderGraphUsdConvertor.h"
#include "PICOSpatialRuntimeSettings.h"

#include "Expressions/PICOSpatialExpressionAdd.h"
#include "Expressions/PICOSpatialExpressionSubtract.h"
#include "Expressions/PICOSpatialExpressionMultiply.h"
#include "Expressions/PICOSpatialExpressionDivide.h"
#include "Expressions/PICOSpatialExpressionPower.h"
#include "Expressions/PICOSpatialExpressionSquareRoot.h"
#include "Expressions/PICOSpatialExpressionDerivative.h"
#include "Expressions/PICOSpatialExpressionFrac.h"
#include "Expressions/PICOSpatialExpressionDistance.h"
#include "Expressions/PICOSpatialExpressionDotProduct.h"
#include "Expressions/PICOSpatialExpressionMinMaxOneMinus.h"
#include "Expressions/PICOSpatialExpressionFloorCeil.h"
#include "Expressions/PICOSpatialExpressionCrossProduct.h"
#include "Expressions/PICOSpatialExpressionClamp.h"
#include "Expressions/PICOSpatialExpressionAbs.h"
#include "Expressions/PICOSpatialExpressionExponential.h"
#include "Expressions/PICOSpatialExpressionModulo.h"
#include "Expressions/PICOSpatialExpressionIf.h"
#include "Expressions/PICOSpatialExpressionLerp.h"
#include "Expressions/PICOSpatialExpressionConstant.h"
#include "Expressions/PICOSpatialExpressionConstant3Vector.h"
#include "Expressions/PICOSpatialExpressionConstant4Vector.h"
#include "Expressions/PICOSpatialExpressionScalarParameter.h"
#include "Expressions/PICOSpatialExpressionVectorParameter.h"
#include "Expressions/PICOSpatialExpressionTextureObject.h"
#include "Expressions/PICOSpatialExpressionTextureSample.h"
#include "Expressions/PICOSpatialExpressionTextureSampleParameter2D.h"
#include "Expressions/PICOSpatialExpressionCameraPosition.h"
#include "Expressions/PICOSpatialExpressionCameraDirection.h"
#include "Expressions/PICOSpatialExpressionNormalize.h"
#include "Expressions/PICOSpatialExpressionWorldPosition.h"
#include "Expressions/PICOSpatialExpressionTime.h"
#include "Expressions/PICOSpatialExpressionTrigonometric.h"
#include "Functions/PICOSpatialExpressionMFPI.h"
#include "Functions/PICOSpatialExpressionMFCameraDirection.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionSine.h"
#include "Materials/MaterialExpressionCosine.h"
#include "Materials/MaterialExpressionArcsine.h"
#include "Materials/MaterialExpressionArcsineFast.h"
#include "Materials/MaterialExpressionArccosine.h"
#include "Materials/MaterialExpressionArccosineFast.h"
#include "Materials/MaterialExpressionTangent.h"
#include "Materials/MaterialExpressionArctangent2.h"
#include "Materials/MaterialExpressionArctangent2Fast.h"
#include "Materials/MaterialExpressionSquareRoot.h"
#include "Materials/MaterialExpressionAbs.h"
#include "Materials/MaterialExpressionExponential.h"
#include "Materials/MaterialExpressionExponential2.h"
#include "Materials/MaterialExpressionLogarithm.h"
#include "Materials/MaterialExpressionLogarithm2.h"
#include "Materials/MaterialExpressionLogarithm10.h"
#include "Materials/MaterialExpressionDDX.h"
#include "Materials/MaterialExpressionDDY.h"
#include "Materials/MaterialExpressionFrac.h"
#include "Materials/MaterialExpressionDistance.h"
#include "Materials/MaterialExpressionDotProduct.h"
#include "Materials/MaterialExpressionMax.h"
#include "Materials/MaterialExpressionMin.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionFloor.h"
#include "Materials/MaterialExpressionCeil.h"
#include "Materials/MaterialExpressionCrossProduct.h"
#include "Materials/MaterialExpressionClamp.h"
#include "Materials/MaterialExpressionFmod.h"
#include "Materials/MaterialExpressionModulo.h"
#include "Materials/MaterialExpressionCameraPositionWS.h"
#include "Materials/MaterialExpressionCameraVectorWS.h"
#include "Materials/MaterialExpressionNormalize.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"

#if USE_USD_SDK
FShaderGraphUsdConvertorPICO::FShaderGraphUsdConvertorPICO(const UMaterialInterface* InMaterialInterface, pxr::UsdStageRefPtr InStage)
	: MaterialInterface(InMaterialInterface)
	, Stage(InStage)
{
	MaterialDebugPath = MaterialInterface ? MaterialInterface->GetPathName() : FString(TEXT("<null>"));
}

FShaderGraphUsdConvertorPICO::~FShaderGraphUsdConvertorPICO()
{
	for (auto& Item : MaterialExpressionToPICONodeMap)
	{
		if (Item.Value)
		{
			delete Item.Value;
		}
	}
}

bool FShaderGraphUsdConvertorPICO::Convert()
{
	UE_LOG(LogPICOSpatialUSD, Display, TEXT("%s[BEGIN][OUT=%s]"), *GetLogPrefix(), *OutputUsdPath);
	LastError.Reset();

	// Step 1: Validate inputs and cast to supported material type
	const UMaterial* Material = ValidateAndCastMaterial();
	if (!Material)
	{
		return Fail(TEXT("Validate"), LastError.IsEmpty() ? TEXT("ValidateAndCastMaterial failed") : LastError);
	}

	EMaterialShadingModel ShadingModel;
	// Step 2: Validate shading model and select conversion branch
	if (!ValidateShadingModel(Material, ShadingModel))
	{
		return Fail(TEXT("ValidateShadingModel"), LastError.IsEmpty() ? TEXT("ValidateShadingModel failed") : LastError);
	}

	// Step 3: Setup USD stage defaults (default prim, units, up axis)
	SetupUsdStageDefaults();
	
	// Step 4: Create USD material prim and write material-level properties
	pxr::UsdShadeMaterial UsdMaterial;
	if (!CreateUsdMaterialWithProperties(Material, UsdMaterial))
	{
		return Fail(TEXT("CreateUsdMaterialWithProperties"), LastError.IsEmpty() ? TEXT("CreateUsdMaterialWithProperties failed") : LastError);
	}
	
	// Step 5: Create surface shader node and connect it to the material surface output
	pxr::UsdShadeShader SurfaceShader;
	if (!CreateSurfaceShader(Material, ShadingModel, UsdMaterial, SurfaceShader))
	{
		return Fail(TEXT("CreateSurfaceShader"), LastError.IsEmpty() ? TEXT("CreateSurfaceShader failed") : LastError);
	}
	
	// Step 6: Create preview geometry and bind the material for visualization
	CreatePreviewGeometry();

	// Step 7: Process material expression nodes (define USD shaders and connect node inputs)
	if (!ProcessMaterialExpressions(Material))
	{
		return Fail(TEXT("ProcessMaterialExpressions"), LastError.IsEmpty() ? TEXT("ProcessMaterialExpressions failed") : LastError);
	}
	
	// Step 8: Connect UE material properties to surface shader inputs
	if (!ConnectMaterialPropertiesToSurface(Material, ShadingModel, SurfaceShader))
	{
		return Fail(TEXT("ConnectMaterialPropertiesToSurface"), LastError.IsEmpty() ? TEXT("ConnectMaterialPropertiesToSurface failed") : LastError);
	}
	
	UE_LOG(LogPICOSpatialUSD, Display, TEXT("%s[OK]"), *GetLogPrefix());
	return true;
}

void FShaderGraphUsdConvertorPICO::SetLogContext(int32 InMaterialSequence, const FString& InOutputUsdPath)
{
	MaterialSequence = InMaterialSequence;
	OutputUsdPath = InOutputUsdPath;
}

FString FShaderGraphUsdConvertorPICO::GetLogPrefix() const
{
	const FString MaterialLabel = MaterialDebugPath.IsEmpty() ? FString(TEXT("<unknown>")) : MaterialDebugPath;
	return FString::Printf(TEXT("[USD_MTLX][#%04d][MAT=%s]"), MaterialSequence, *MaterialLabel);
}

void FShaderGraphUsdConvertorPICO::SetFailure(const FString& Step, const FString& Reason)
{
	if (!Step.IsEmpty())
	{
		LastError = FString::Printf(TEXT("[%s] %s"), *Step, *Reason);
	}
	else
	{
		LastError = Reason;
	}
}

bool FShaderGraphUsdConvertorPICO::Fail(const FString& Step, const FString& Reason)
{
	SetFailure(Step, Reason);
	UE_LOG(LogPICOSpatialUSD, Warning, TEXT("%s[FAIL][%s] %s"), *GetLogPrefix(), *Step, *Reason);
	return false;
}

ENodeTypePICO FShaderGraphUsdConvertorPICO::GetExpressionType(const UMaterialExpression* Expression)
{
	if (!Expression)
	{
		return ENodeTypePICO::MNP_Unsupported;
	}

	if (Expression->IsA(UMaterialExpressionAdd::StaticClass()))
	{
		return ENodeTypePICO::MNP_Add;
	}
	else if (Expression->IsA(UMaterialExpressionSubtract::StaticClass()))
	{
		return ENodeTypePICO::MNP_Subtract;
	}
	else if (Expression->IsA(UMaterialExpressionMultiply::StaticClass()))
	{
		return ENodeTypePICO::MNP_Multiply;
	}
	else if (Expression->IsA(UMaterialExpressionDivide::StaticClass()))
	{
		return ENodeTypePICO::MNP_Divide;
	}
	else if (Expression->IsA(UMaterialExpressionConstant3Vector::StaticClass()))
	{
		return ENodeTypePICO::MNP_Constant3Vector;
	}
	else if (Expression->IsA(UMaterialExpressionConstant::StaticClass()))
	{
		return ENodeTypePICO::MNP_Constant;
	}
	else if (Expression->IsA(UMaterialExpressionConstant4Vector::StaticClass()))
	{
		return ENodeTypePICO::MNP_Constant4Vector;
	}
	else if (Expression->IsA(UMaterialExpressionScalarParameter::StaticClass()))
	{
		return ENodeTypePICO::MNP_ScalarParameter;
	}
	else if (Expression->IsA(UMaterialExpressionVectorParameter::StaticClass()))
	{
		return ENodeTypePICO::MNP_VectorParameter;
	}
	else if (Expression->IsA(UMaterialExpressionTextureObject::StaticClass()))
	{
		return ENodeTypePICO::MNP_TextureObject;
	}
	else if (Expression->IsA(UMaterialExpressionCameraPositionWS::StaticClass()))
	{
		return ENodeTypePICO::MNP_CameraPosition;
	}
	else if (Expression->IsA(UMaterialExpressionCameraVectorWS::StaticClass()))
	{
		return ENodeTypePICO::MNP_CameraDirection;
	}
	else if (Expression->IsA(UMaterialExpressionNormalize::StaticClass()))
	{
		return ENodeTypePICO::MNP_Normalize;
	}
	else if (Expression->IsA(UMaterialExpressionWorldPosition::StaticClass()))
	{
		return ENodeTypePICO::MNP_WorldPosition;
	}
	else if (Expression->IsA(UMaterialExpressionTextureSampleParameter2D::StaticClass()))
	{
		return ENodeTypePICO::MNP_TextureSampleParameter2D;
	}
	else if (Expression->IsA(UMaterialExpressionTextureSample::StaticClass()))
	{
		return ENodeTypePICO::MNP_TextureSample;
	}
	else if (Expression->IsA(UMaterialExpressionPower::StaticClass()))
	{
		return ENodeTypePICO::MNP_Power;
	}
	else if (Expression->IsA(UMaterialExpressionSquareRoot::StaticClass()))
	{
		return ENodeTypePICO::MNP_Sqrt;
	}
	else if (Expression->IsA(UMaterialExpressionAbs::StaticClass()))
	{
		return ENodeTypePICO::MNP_Abs;
	}
	else if (Expression->IsA(UMaterialExpressionExponential::StaticClass()))
	{
		return ENodeTypePICO::MNP_Exp;
	}
	else if (Expression->IsA(UMaterialExpressionExponential2::StaticClass()))
	{
		return ENodeTypePICO::MNP_Exp2;
	}
	else if (Expression->IsA(UMaterialExpressionLogarithm::StaticClass()))
	{
		return ENodeTypePICO::MNP_Log;
	}
	else if (Expression->IsA(UMaterialExpressionLogarithm2::StaticClass()))
	{
		return ENodeTypePICO::MNP_Log2;
	}
	else if (Expression->IsA(UMaterialExpressionLogarithm10::StaticClass()))
	{
		return ENodeTypePICO::MNP_Log10;
	}
	else if (Expression->IsA(UMaterialExpressionDDX::StaticClass()))
	{
		return ENodeTypePICO::MNP_DDX;
	}
	else if (Expression->IsA(UMaterialExpressionDDY::StaticClass()))
	{
		return ENodeTypePICO::MNP_DDY;
	}
	else if (Expression->IsA(UMaterialExpressionFrac::StaticClass()))
	{
		return ENodeTypePICO::MNP_Frac;
	}
	else if (Expression->IsA(UMaterialExpressionDistance::StaticClass()))
	{
		return ENodeTypePICO::MNP_Distance;
	}
	else if (Expression->IsA(UMaterialExpressionDotProduct::StaticClass()))
	{
		return ENodeTypePICO::MNP_DotProduct;
	}
	else if (Expression->IsA(UMaterialExpressionMax::StaticClass()))
	{
		return ENodeTypePICO::MNP_Max;
	}
	else if (Expression->IsA(UMaterialExpressionMin::StaticClass()))
	{
		return ENodeTypePICO::MNP_Min;
	}
	else if (Expression->IsA(UMaterialExpressionOneMinus::StaticClass()))
	{
		return ENodeTypePICO::MNP_OneMinus;
	}
	else if (Expression->IsA(UMaterialExpressionFloor::StaticClass()))
	{
		return ENodeTypePICO::MNP_Floor;
	}
	else if (Expression->IsA(UMaterialExpressionCeil::StaticClass()))
	{
		return ENodeTypePICO::MNP_Ceil;
	}
	else if (Expression->IsA(UMaterialExpressionCrossProduct::StaticClass()))
	{
		return ENodeTypePICO::MNP_CrossProduct;
	}
	else if (Expression->IsA(UMaterialExpressionClamp::StaticClass()))
	{
		return ENodeTypePICO::MNP_Clamp;
	}
	else if (Expression->IsA(UMaterialExpressionFmod::StaticClass()))
	{
		return ENodeTypePICO::MNP_Modulo;
	}
	else if (Expression->IsA(UMaterialExpressionModulo::StaticClass()))
	{
		return ENodeTypePICO::MNP_Modulo;
	}
	else if (Expression->IsA(UMaterialExpressionIf::StaticClass()))
	{
		return ENodeTypePICO::MNP_If;
	}
	else if (Expression->IsA(UMaterialExpressionLinearInterpolate::StaticClass()))
	{
		return ENodeTypePICO::MNP_Lerp;
	}
	else if (Expression->IsA(UMaterialExpressionSine::StaticClass()))
	{
		return ENodeTypePICO::MNP_Sine;
	}
	else if (Expression->IsA(UMaterialExpressionCosine::StaticClass()))
	{
		return ENodeTypePICO::MNP_Cosine;
	}
	else if (Expression->IsA(UMaterialExpressionArcsine::StaticClass()))
	{
		return ENodeTypePICO::MNP_Arcsine;
	}
	else if (Expression->IsA(UMaterialExpressionArcsineFast::StaticClass()))
	{
		return ENodeTypePICO::MNP_Arcsine;
	}
	else if (Expression->IsA(UMaterialExpressionArccosine::StaticClass()))
	{
		return ENodeTypePICO::MNP_Arccosine;
	}
	else if (Expression->IsA(UMaterialExpressionArccosineFast::StaticClass()))
	{
		return ENodeTypePICO::MNP_Arccosine;
	}
	else if (Expression->IsA(UMaterialExpressionTangent::StaticClass()))
	{
		return ENodeTypePICO::MNP_Tangent;
	}
	else if (Expression->IsA(UMaterialExpressionArctangent2::StaticClass()))
	{
		return ENodeTypePICO::MNP_Arctangent2;
	}
	else if (Expression->IsA(UMaterialExpressionArctangent2Fast::StaticClass()))
	{
		return ENodeTypePICO::MNP_Arctangent2;
	}
	else if (Expression->IsA(UMaterialExpressionTime::StaticClass()))
	{
		return ENodeTypePICO::MNP_Time;
	}
	else if (Expression->IsA(UMaterialExpressionMaterialFunctionCall::StaticClass()))
	{
		const UMaterialExpressionMaterialFunctionCall* Call = Cast<UMaterialExpressionMaterialFunctionCall>(Expression);
		const UMaterialFunctionInterface* Function = Call ? Call->MaterialFunction : nullptr;
		const FString FunctionNameNoSpace = Function ? Function->GetName().Replace(TEXT(" "), TEXT("")) : FString();

		const bool bIsPi = FunctionNameNoSpace.Equals(TEXT("PI"), ESearchCase::IgnoreCase);
		if (bIsPi)
		{
			return ENodeTypePICO::MNP_MF_PI;
		}

		const bool bIsCameraDirectionVector = FunctionNameNoSpace.Equals(TEXT("CameraDirectionVector"), ESearchCase::IgnoreCase)
			|| FunctionNameNoSpace.Equals(TEXT("CameraDirection"), ESearchCase::IgnoreCase);
		if (bIsCameraDirectionVector)
		{
			return ENodeTypePICO::MNP_MF_CameraDirection;
		}
	}

	return ENodeTypePICO::MNP_Unsupported;
}

FExpressionBasePICO* FShaderGraphUsdConvertorPICO::CreateExpressionPICO(UMaterialExpression* Expression)
{
	if (!Expression)
	{
		return nullptr;
	}

	ENodeTypePICO NodeType = GetExpressionType(Expression);

	switch (NodeType)
	{
	case MNP_Unsupported:
		break;
	case MNP_Constant:
		return new FExpressionConstantPICO(this, Expression);
		break;
	case MNP_Constant2Vector:
		break;
	case MNP_Constant3Vector:
		return new FExpressionConstant3VectorPICO(this, Expression);
		break;
	case MNP_Constant4Vector:
		return new FExpressionConstant4VectorPICO(this, Expression);
		break;
	case MNP_TextureSample:
		return new FExpressionTextureSamplePICO(this, Expression);
		break;
	case MNP_TextureSampleParameter2D:
		return new FExpressionTextureSampleParameter2DPICO(this, Expression);
		break;
	case MNP_TextureCoordinate:
		break;
	case MNP_TextureObject:
		return new FExpressionTextureObjectPICO(this, Expression);
		break;
	case MNP_CameraPosition:
		return new FExpressionCameraPositionPICO(this, Expression);
		break;
	case MNP_CameraDirection:
		return new FExpressionCameraDirectionPICO(this, Expression);
		break;
	case MNP_Normalize:
		return new FExpressionNormalizePICO(this, Expression);
		break;
	case MNP_WorldPosition:
		return new FExpressionWorldPositionPICO(this, Expression);
		break;
	case MNP_Multiply:
		return new FExpressionMultiplyPICO(this, Expression);
		break;
	case MNP_Add:
		return new FExpressionAddPICO(this, Expression);
		break;
	case MNP_Lerp:
		return new FExpressionLerpPICO(this, Expression);
		break;
	case MNP_Sine:
		return new FExpressionSinePICO(this, Expression);
		break;
	case MNP_Cosine:
		return new FExpressionCosinePICO(this, Expression);
		break;
	case MNP_Arcsine:
		return new FExpressionArcsinePICO(this, Expression);
		break;
	case MNP_Arccosine:
		return new FExpressionArccosinePICO(this, Expression);
		break;
	case MNP_Tangent:
		return new FExpressionTangentPICO(this, Expression);
		break;
	case MNP_Arctangent2:
		return new FExpressionArctangent2PICO(this, Expression);
		break;
	case MNP_Time:
		return new FExpressionTimePICO(this, Expression);
		break;
	case MNP_MF_PI:
		return new FExpressionMFPIPICO(this, Expression);
		break;
	case MNP_MF_CameraDirection:
		return new FExpressionMFCameraDirectionPICO(this, Expression);
		break;
	case MNP_Power:
		return new FExpressionPowerPICO(this, Expression);
		break;
	case MNP_Sqrt:
		return new FExpressionSquareRootPICO(this, Expression);
		break;
	case MNP_Abs:
		return new FExpressionAbsPICO(this, Expression);
		break;
	case MNP_Exp:
		return new FExpressionExponentialPICO(this, Expression);
		break;
	case MNP_Exp2:
		return new FExpressionExponential2PICO(this, Expression);
		break;
	case MNP_Log:
		return new FExpressionLogarithmPICO(this, Expression);
		break;
	case MNP_Log2:
		return new FExpressionLogarithm2PICO(this, Expression);
		break;
	case MNP_Log10:
		return new FExpressionLogarithm10PICO(this, Expression);
		break;
	case MNP_DDX:
		return new FExpressionDDXPICO(this, Expression);
		break;
	case MNP_DDY:
		return new FExpressionDDYPICO(this, Expression);
		break;
	case MNP_Frac:
		return new FExpressionFracPICO(this, Expression);
		break;
	case MNP_Distance:
		return new FExpressionDistancePICO(this, Expression);
		break;
	case MNP_DotProduct:
		return new FExpressionDotProductPICO(this, Expression);
		break;
	case MNP_Max:
		return new FExpressionMaxPICO(this, Expression);
		break;
	case MNP_Min:
		return new FExpressionMinPICO(this, Expression);
		break;
	case MNP_OneMinus:
		return new FExpressionOneMinusPICO(this, Expression);
		break;
	case MNP_Floor:
		return new FExpressionFloorPICO(this, Expression);
		break;
	case MNP_Ceil:
		return new FExpressionCeilPICO(this, Expression);
		break;
	case MNP_CrossProduct:
		return new FExpressionCrossProductPICO(this, Expression);
		break;
	case MNP_Clamp:
		return new FExpressionClampPICO(this, Expression);
		break;
	case MNP_Modulo:
		return new FExpressionModuloPICO(this, Expression);
		break;
	case MNP_If:
		return new FExpressionIfPICO(this, Expression);
		break;
	case MNP_Divide:
		return new FExpressionDividePICO(this, Expression);
		break;
	case MNP_Subtract:
		return new FExpressionSubtractPICO(this, Expression);
		break;
	case MNP_ScalarParameter:
		return new FExpressionScalarParameterPICO(this, Expression);
		break;
	case MNP_VectorParameter:
		return new FExpressionVectorParameterPICO(this, Expression);
		break;
	case MNP_StaticSwitchParameter:
		break;
	case MNP_RootNode:
		break;
	default:
		break;
	}

	return nullptr;
}

EMaterialValueType FShaderGraphUsdConvertorPICO::InferenceInputType(UMaterialExpression* Expression, int InputIndex)
{
	if (!Expression)
	{
		return EMaterialValueType::MCT_Unknown;
	}

	if (FExpressionInput* Input = Expression->GetInput(InputIndex))
	{
		if (Input->IsConnected() && Input->Expression)
		{
			FExpressionBasePICO** Node = MaterialExpressionToPICONodeMap.Find(Input->Expression);
			if (Node)
			{
				//return (*Node)->GetOutputType(Input->OutputIndex);
			}
		}
	}

	return EMaterialValueType::MCT_Unknown;
}

FExpressionBasePICO* FShaderGraphUsdConvertorPICO::FindNode(UMaterialExpression* Expression)
{
	FExpressionBasePICO** NodeFound = MaterialExpressionToPICONodeMap.Find(Expression);
	if (NodeFound)
	{
		return *NodeFound;
	}
	return nullptr;
}

// ---------------------------------------------------------------------------
// Forward type propagation – file-local helpers
// ---------------------------------------------------------------------------
namespace
{
	pxr::SdfValueTypeName MaxUsdType(const pxr::SdfValueTypeName& A, const pxr::SdfValueTypeName& B)
	{
		return FShaderGraphUsdConvertorPICO::GetTypeRank(A) >= FShaderGraphUsdConvertorPICO::GetTypeRank(B) ? A : B;
	}

	// Returns the statically-known USD type for a leaf node, or {} for non-leaf nodes.
	pxr::SdfValueTypeName GetStaticLeafOutputType(ENodeTypePICO NodeType)
	{
		switch (NodeType)
		{
		case MNP_Constant:              return pxr::SdfValueTypeNames->Float;
		case MNP_Constant2Vector:       return pxr::SdfValueTypeNames->Float2;
		case MNP_Constant3Vector:       return pxr::SdfValueTypeNames->Color3f;
		case MNP_Constant4Vector:       return pxr::SdfValueTypeNames->Color4f;
		case MNP_TextureCoordinate:     return pxr::SdfValueTypeNames->Float2;
		case MNP_ScalarParameter:       return pxr::SdfValueTypeNames->Float;
		case MNP_VectorParameter:       return pxr::SdfValueTypeNames->Color4f;
		case MNP_CameraPosition:        return pxr::SdfValueTypeNames->Float3;
		case MNP_CameraDirection:       return pxr::SdfValueTypeNames->Float3;
		case MNP_WorldPosition:         return pxr::SdfValueTypeNames->Float3;
		case MNP_Time:                  return pxr::SdfValueTypeNames->Float;
		case MNP_MF_PI:                 return pxr::SdfValueTypeNames->Float;
		case MNP_MF_CameraDirection:    return pxr::SdfValueTypeNames->Float3;
		// TextureObject outputs a filename token, not a numeric type;
		// it is only ever wired into a TextureSample's Texture slot, never into
		// a numeric math node, so Float is a safe sentinel that keeps it out of
		// the max-type promotion logic.
		case MNP_TextureObject:         return pxr::SdfValueTypeNames->Float;
		// TextureSample has per-pin types; use Color4f as a representative sentinel.
		case MNP_TextureSample:
		case MNP_TextureSampleParameter2D:
			return pxr::SdfValueTypeNames->Color4f;
		default:
			return {}; // non-leaf: type must be inferred
		}
	}

	// Returns the **output type** that flows out of `Node`'s output at `OutputIndex`
	// for consumption by downstream nodes during type propagation.
	// This is distinct from InferredUsdType (the operating type):
	//   - For most nodes: output type == operating type (add, multiply, normalize, ...)
	//   - For scalar-reduction nodes (magnitude, distance, dotproduct): output is
	//     always Float regardless of operating type.
	// Requires topological order so that non-leaf nodes already have InferredUsdType set.
	// IMPORTANT: when adding a new scalar-reduction node (e.g. MNP_Magnitude),
	// add a case below that returns Float instead of Node->InferredUsdType.
	pxr::SdfValueTypeName GetNodeOutputTypeForPropagation(
		const FExpressionBasePICO* Node, int32 OutputIndex)
	{
		if (Node->NodeType == MNP_Distance || Node->NodeType == MNP_DotProduct)
		{
			return pxr::SdfValueTypeNames->Float;
		}
		if (Node->NodeType == MNP_TextureSample ||
			Node->NodeType == MNP_TextureSampleParameter2D)
		{
			// Index 0 = RGB Color3f, 1-4 = R/G/B/A Float, 5 = RGBA Color4f
			if (OutputIndex == 5) return pxr::SdfValueTypeNames->Color4f;
			if (OutputIndex == 0) return pxr::SdfValueTypeNames->Color3f;
			return pxr::SdfValueTypeNames->Float;
		}
		pxr::SdfValueTypeName Leaf = GetStaticLeafOutputType(Node->NodeType);
		if (Leaf) return Leaf;
		// For current nodes InferredUsdType == operating type == output type.
		// Scalar-reduction nodes must be handled above.
		return Node->InferredUsdType; // already propagated in the current pass
	}

    bool IsMaterialPropertyActiveForExport(const UMaterial* Material, EMaterialProperty Property)
    {
            if (!Material)
            {
                    return false;
            }
#if WITH_EDITOR
            return Material->IsPropertyActiveInEditor(Property);
#else
            return Material->IsPropertyActive(Property);
#endif
    }
} // anonymous namespace

// ---------------------------------------------------------------------------
// FShaderGraphUsdConvertorPICO: RunForwardTypePropagation
// ---------------------------------------------------------------------------
void FShaderGraphUsdConvertorPICO::RunForwardTypePropagation()
{
	// ---- Step A: topological sort via post-order DFS ----
	TArray<FExpressionBasePICO*> SortedNodes;
	TSet<UMaterialExpression*> Visited;

	TFunction<void(FExpressionBasePICO*)> DFS = [&](FExpressionBasePICO* Node)
	{
		if (!Node || Visited.Contains(Node->Owner)) return;
		Visited.Add(Node->Owner);

		const int32 NumInputs = Node->Owner->CountInputs();
		for (int32 i = 0; i < NumInputs; ++i)
		{
			FExpressionInput* Input = Node->Owner->GetInput(i);
			if (!Input || !Input->IsConnected() || !Input->Expression) continue;
			FExpressionBasePICO** UpNode = MaterialExpressionToPICONodeMap.Find(Input->Expression);
			if (UpNode && *UpNode) DFS(*UpNode);
		}
		SortedNodes.Add(Node);
	};

	for (auto& Item : MaterialExpressionToPICONodeMap)
	{
		DFS(Item.Value);
	}

	// ---- Step B: propagate types in topological order ----
	for (FExpressionBasePICO* Node : SortedNodes)
	{
		// Leaf nodes have a fixed, statically-known type.
		const pxr::SdfValueTypeName LeafType = GetStaticLeafOutputType(Node->NodeType);
		if (LeafType)
		{
			Node->InferredUsdType = LeafType;
			continue;
		}

		// Non-leaf: determine operating type via ResolveOperatingType().
		//
		// Step 1 - compute desired type = max() of participating upstream outputs.
		//   If no input is connected, DesiredType stays empty; we'll pass Float below.
		// Step 2 - ResolveOperatingType maps desired - the minimum-dimension supported
		//   variant whose dim >= desired dim, with color preferred over vector at the
		//   same dimension.  If desired exceeds every supported variant, the largest
		//   supported variant is returned and the connect phase will insert a convert
		//   node to downcast the upstream output.
		//
		// Fixed-float semantic inputs must NOT participate in the desired-type max:
		//   - MNP_Lerp input 2 (Alpha): `mix` float channel.
		//   - MNP_If inputs 0 (A) and 1 (B): float comparison thresholds.
		pxr::SdfValueTypeName DesiredType; // empty = no upstream constraint yet
		const int32 NumInputs = Node->Owner->CountInputs();
		for (int32 i = 0; i < NumInputs; ++i)
		{
			// Skip fixed-float inputs that don't contribute to the primary type.
			if (Node->NodeType == MNP_Lerp && i == 2) continue;
			if (Node->NodeType == MNP_If && (i == 0 || i == 1)) continue;

			FExpressionInput* Input = Node->Owner->GetInput(i);
			if (!Input || !Input->IsConnected() || !Input->Expression) continue;

			FExpressionBasePICO** UpNode = MaterialExpressionToPICONodeMap.Find(Input->Expression);
			if (!UpNode || !*UpNode) continue;

			pxr::SdfValueTypeName UpType =
				GetNodeOutputTypeForPropagation(*UpNode, Input->OutputIndex);
			if (UpType)
			{
				DesiredType = MaxUsdType(DesiredType, UpType);
			}
		}

		// Pass Float when nothing is connected (scalar constant defaults).
		Node->InferredUsdType = Node->ResolveOperatingType(
			DesiredType ? DesiredType : pxr::SdfValueTypeNames->Float);
	}
}

// ---------------------------------------------------------------------------
// FShaderGraphUsdConvertorPICO: static type helpers
// ---------------------------------------------------------------------------
int32 FShaderGraphUsdConvertorPICO::GetTypeRank(const pxr::SdfValueTypeName& Type)
{
	if (Type == pxr::SdfValueTypeNames->Color4f) return 8;
	if (Type == pxr::SdfValueTypeNames->Float4)  return 7;
	if (Type == pxr::SdfValueTypeNames->Color3f) return 6;
	if (Type == pxr::SdfValueTypeNames->Float3)  return 5;
	if (Type == pxr::SdfValueTypeNames->Float2)  return 4;
	if (Type == pxr::SdfValueTypeNames->Float)   return 1;
	return 0;
}

int32 FShaderGraphUsdConvertorPICO::GetTypeDimension(const pxr::SdfValueTypeName& Type)
{
	if (Type == pxr::SdfValueTypeNames->Float)                                     return 1;
	if (Type == pxr::SdfValueTypeNames->Float2)                                    return 2;
	if (Type == pxr::SdfValueTypeNames->Float3 ||
		Type == pxr::SdfValueTypeNames->Color3f)                                   return 3;
	if (Type == pxr::SdfValueTypeNames->Float4 ||
		Type == pxr::SdfValueTypeNames->Color4f)                                   return 4;
	return 0;
}

FString FShaderGraphUsdConvertorPICO::GetMxTypeSuffix(const pxr::SdfValueTypeName& Type)
{
	if (Type == pxr::SdfValueTypeNames->Float)   return TEXT("float");
	if (Type == pxr::SdfValueTypeNames->Float2)  return TEXT("vector2");
	if (Type == pxr::SdfValueTypeNames->Float3)  return TEXT("vector3");
	if (Type == pxr::SdfValueTypeNames->Float4)  return TEXT("vector4");
	if (Type == pxr::SdfValueTypeNames->Color3f) return TEXT("color3");
	if (Type == pxr::SdfValueTypeNames->Color4f) return TEXT("color4");
	return TEXT("color4"); // safe fallback
}

bool FShaderGraphUsdConvertorPICO::IsColorUsdType(const pxr::SdfValueTypeName& Type)
{
	return Type == pxr::SdfValueTypeNames->Color3f ||
		   Type == pxr::SdfValueTypeNames->Color4f;
}

void FShaderGraphUsdConvertorPICO::SetDefaultInputValue(
	pxr::UsdShadeInput& Input, float DefaultFloat, const pxr::SdfValueTypeName& Type)
{
	if (Type == pxr::SdfValueTypeNames->Float)
		Input.Set(DefaultFloat);
	else if (Type == pxr::SdfValueTypeNames->Float2)
		Input.Set(pxr::GfVec2f(DefaultFloat, DefaultFloat));
	else if (Type == pxr::SdfValueTypeNames->Float3 ||
			 Type == pxr::SdfValueTypeNames->Color3f)
		Input.Set(pxr::GfVec3f(DefaultFloat, DefaultFloat, DefaultFloat));
	else
		Input.Set(pxr::GfVec4f(DefaultFloat, DefaultFloat, DefaultFloat, DefaultFloat));
}

pxr::SdfValueTypeName FShaderGraphUsdConvertorPICO::ToVectorEquivalent(
	const pxr::SdfValueTypeName& Type)
{
	if (Type == pxr::SdfValueTypeNames->Color3f) return pxr::SdfValueTypeNames->Float3;
	if (Type == pxr::SdfValueTypeNames->Color4f) return pxr::SdfValueTypeNames->Float4;
	return Type;
}

FExpressionBasePICO::FExpressionBasePICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
	: Convertor(InConvertor)
	, Owner(InExpression)
	, bDefinedUsdShader(false)
{
	check(InConvertor);
	check(InExpression);
	bIsParameterNode = InExpression->bIsParameterExpression;
	bIsSupportedNode = (NodeType = Convertor->GetExpressionType(InExpression)) != ENodeTypePICO::MNP_Unsupported;
	NodeName = InExpression->GetName();
}

FExpressionBasePICO::~FExpressionBasePICO()
{
}

pxr::SdfValueTypeName FExpressionBasePICO::GetInputType(int Index)
{
	pxr::SdfValueTypeName Name;
	if (pxr::UsdShadeInput* ShadeInput = ShaderInputs.Find(Index))
	{
		Name = (*ShadeInput).GetTypeName();
	}
	return Name;
}

pxr::SdfValueTypeName FExpressionBasePICO::GetOutputType(int Index)
{
	pxr::SdfValueTypeName Name;
	if (pxr::UsdShadeOutput* ShadeOutput = ShaderOutputs.Find(Index))
	{
		Name = (*ShadeOutput).GetTypeName();
	}
	return Name;
}

// Default variant list: all standard float / color types, ordered dim-ascending.
// Color is listed before vector at the same dimension so ResolveOperatingType()
// naturally prefers color when both are present.
TArray<pxr::SdfValueTypeName> FExpressionBasePICO::GetSupportedVariants() const
{
	return TArray<pxr::SdfValueTypeName>({
		pxr::SdfValueTypeNames->Float,    // dim 1
		pxr::SdfValueTypeNames->Float2,   // dim 2
		pxr::SdfValueTypeNames->Color3f,  // dim 3 color preferred
		pxr::SdfValueTypeNames->Float3,   // dim 3 vector fallback
		pxr::SdfValueTypeNames->Color4f,  // dim 4 color preferred
		pxr::SdfValueTypeNames->Float4,   // dim 4 vector fallback
	});
}

pxr::SdfValueTypeName FExpressionBasePICO::ResolveOperatingType(
	const pxr::SdfValueTypeName& DesiredType) const
{
	const TArray<pxr::SdfValueTypeName> Variants = GetSupportedVariants();
	if (Variants.IsEmpty())
	{
		return DesiredType;
	}

	const int32 DesiredDim = FShaderGraphUsdConvertorPICO::GetTypeDimension(DesiredType);

	// Pass 1: find the minimum-dimension variant that can accommodate DesiredDim.
	// Among variants at the same (minimum qualifying) dimension, color wins over vector.
	pxr::SdfValueTypeName Best;
	int32 BestDim = INT_MAX;
	bool bBestIsColor = false;

	for (const pxr::SdfValueTypeName& V : Variants)
	{
		const int32 Dim = FShaderGraphUsdConvertorPICO::GetTypeDimension(V);
		if (Dim < DesiredDim) continue; // too small

		const bool bIsColor = FShaderGraphUsdConvertorPICO::IsColorUsdType(V);

		if (!Best || Dim < BestDim || (Dim == BestDim && bIsColor && !bBestIsColor))
		{
			Best = V;
			BestDim = Dim;
			bBestIsColor = bIsColor;
		}
	}

	if (Best)
	{
		return Best;
	}

	// Pass 2: desired dim exceeds every supported variant.
	// Clamp to the largest supported variant; the connect phase will insert a
	// convert node to downcast the upstream output to this type.
	pxr::SdfValueTypeName MaxVariant;
	int32 MaxDim = -1;
	bool bMaxIsColor = false;
	for (const pxr::SdfValueTypeName& V : Variants)
	{
		const int32 Dim = FShaderGraphUsdConvertorPICO::GetTypeDimension(V);
		const bool bIsColor = FShaderGraphUsdConvertorPICO::IsColorUsdType(V);
		if (!MaxVariant || Dim > MaxDim || (Dim == MaxDim && bIsColor && !bMaxIsColor))
		{
			MaxVariant = V;
			MaxDim = Dim;
			bMaxIsColor = bIsColor;
		}
	}
	return MaxVariant;
}

FString FShaderGraphUsdConvertorPICO::GenerateUniqueShaderPath(const FString& DesiredPath)
{
	check(Stage);
	
	pxr::SdfPath UsdPath = pxr::SdfPath(TCHAR_TO_UTF8(*DesiredPath));
	pxr::UsdPrim ExistingPrim = Stage->GetPrimAtPath(UsdPath);
	
	// If path doesn't exist, use it directly
	if (!ExistingPrim.IsValid())
	{
		return DesiredPath;
	}
	
	// Path exists, try to generate unique path with random suffix
	int TryCounter = 0;
	while (TryCounter < 10)
	{
		int32 RandomSuffix = FMath::RandRange(100000, 999999);
		FString UniquePath = FString::Printf(TEXT("%s_%d"), *DesiredPath, RandomSuffix);
		pxr::SdfPath UniqueUsdPath = pxr::SdfPath(TCHAR_TO_UTF8(*UniquePath));
		
		pxr::UsdPrim TestPrim = Stage->GetPrimAtPath(UniqueUsdPath);
		if (!TestPrim.IsValid())
		{
			UE_LOG(LogPICOSpatialUSD, Warning,
				TEXT("Shader path '%s' already exists, using unique path '%s'"),
				*DesiredPath, *UniquePath);
			return UniquePath;
		}
		
		++TryCounter;
	}
	
	UE_LOG(LogPICOSpatialUSD, Error,
		TEXT("Failed to generate unique shader path for '%s' after multiple attempts"),
		*DesiredPath);
	return TEXT("");
}

pxr::UsdShadeShader FShaderGraphUsdConvertorPICO::CreateConvertOrSwizzleShader(pxr::SdfValueTypeName FromType, pxr::SdfValueTypeName ToType)
{
	check(Stage);

	if (!FromType || !ToType || FromType == ToType)
	{
		return pxr::UsdShadeShader();
	}

	// Generate unique path for conversion shader
	FString ConvertShaderPath = GenerateUniqueShaderPath("/Root/Material/Convert");
	if (ConvertShaderPath.IsEmpty())
	{
		UE_LOG(LogPICOSpatialUSD, Error,
			TEXT("Failed to create conversion shader from %s to %s: Unable to generate unique shader path after multiple attempts"),
			UTF8_TO_TCHAR(FromType.GetAsToken().GetString().c_str()),
			UTF8_TO_TCHAR(ToType.GetAsToken().GetString().c_str()));
		return pxr::UsdShadeShader();
	}

	pxr::UsdShadeShader ConvertShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ConvertShaderPath)));
	
	// Determine conversion type and setup shader attributes
	FString ConversionType;

	auto IsColorType = [](const pxr::SdfValueTypeName& Type) -> bool
	{
		return Type == pxr::SdfValueTypeNames->Color3f || Type == pxr::SdfValueTypeNames->Color4f;
	};

	auto GetMxTypeName = [](const pxr::SdfValueTypeName& Type) -> FString
	{
		if (Type == pxr::SdfValueTypeNames->Float) return TEXT("float");
		if (Type == pxr::SdfValueTypeNames->Int) return TEXT("integer");
		if (Type == pxr::SdfValueTypeNames->Bool) return TEXT("boolean");
		if (Type == pxr::SdfValueTypeNames->Color3f) return TEXT("color3");
		if (Type == pxr::SdfValueTypeNames->Color4f) return TEXT("color4");
		if (Type == pxr::SdfValueTypeNames->Float2) return TEXT("vector2");
		if (Type == pxr::SdfValueTypeNames->Float3) return TEXT("vector3");
		if (Type == pxr::SdfValueTypeNames->Float4) return TEXT("vector4");
		return TEXT("");
	};

	auto GetChannelCount = [](const pxr::SdfValueTypeName& Type) -> int32
	{
		if (Type == pxr::SdfValueTypeNames->Color3f || Type == pxr::SdfValueTypeNames->Float3) return 3;
		if (Type == pxr::SdfValueTypeNames->Color4f || Type == pxr::SdfValueTypeNames->Float4) return 4;
		if (Type == pxr::SdfValueTypeNames->Float2) return 2;
		return 1;
	};

	auto CreateConvert = [&](const FString& Id, const pxr::SdfValueTypeName& InType, const pxr::SdfValueTypeName& OutType) -> void
	{
		ConvertShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*Id)));
		pxr::UsdShadeInput In = ConvertShader.CreateInput(pxr::TfToken("in"), InType);
		if (IsColorType(InType))
		{
			In.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
		}
		ConvertShader.CreateOutput(pxr::TfToken("out"), OutType);
		ConversionType = FString::Printf(TEXT("%s_%s"), *GetMxTypeName(InType), *GetMxTypeName(OutType));
	};

	auto HasConvertNodedef = [&](const pxr::SdfValueTypeName& InType, const pxr::SdfValueTypeName& OutType) -> bool
	{
		if (InType == pxr::SdfValueTypeNames->Float)
		{
			return OutType == pxr::SdfValueTypeNames->Color3f
				|| OutType == pxr::SdfValueTypeNames->Color4f
				|| OutType == pxr::SdfValueTypeNames->Float2
				|| OutType == pxr::SdfValueTypeNames->Float3
				|| OutType == pxr::SdfValueTypeNames->Float4
				|| OutType == pxr::SdfValueTypeNames->Int;
		}

		if (InType == pxr::SdfValueTypeNames->Float2)
		{
			return OutType == pxr::SdfValueTypeNames->Float3;
		}

		if (InType == pxr::SdfValueTypeNames->Float3)
		{
			return OutType == pxr::SdfValueTypeNames->Float2
				|| OutType == pxr::SdfValueTypeNames->Float4
				|| OutType == pxr::SdfValueTypeNames->Color3f;
		}

		if (InType == pxr::SdfValueTypeNames->Float4)
		{
			return OutType == pxr::SdfValueTypeNames->Float3
				|| OutType == pxr::SdfValueTypeNames->Color4f;
		}

		if (InType == pxr::SdfValueTypeNames->Color3f)
		{
			return OutType == pxr::SdfValueTypeNames->Float3
				|| OutType == pxr::SdfValueTypeNames->Color4f;
		}

		if (InType == pxr::SdfValueTypeNames->Color4f)
		{
			return OutType == pxr::SdfValueTypeNames->Float4
				|| OutType == pxr::SdfValueTypeNames->Color3f;
		}

		if (InType == pxr::SdfValueTypeNames->Bool)
		{
			return OutType == pxr::SdfValueTypeNames->Float;
		}

		if (InType == pxr::SdfValueTypeNames->Int)
		{
			return OutType == pxr::SdfValueTypeNames->Float;
		}

		return false;
	};

	auto CreateSwizzle = [&](const FString& Id, const pxr::SdfValueTypeName& InType, const pxr::SdfValueTypeName& OutType) -> bool
	{
		const FString InMx = GetMxTypeName(InType);
		const FString OutMx = GetMxTypeName(OutType);
		if (InMx.IsEmpty() || OutMx.IsEmpty())
		{
			return false;
		}

		if (HasConvertNodedef(InType, OutType))
		{
			return false;
		}

		if (InType == pxr::SdfValueTypeNames->Int || InType == pxr::SdfValueTypeNames->Bool || OutType == pxr::SdfValueTypeNames->Int || OutType == pxr::SdfValueTypeNames->Bool)
		{
			return false;
		}

		ConvertShader.CreateIdAttr().Set(pxr::TfToken(TCHAR_TO_UTF8(*Id)));
		pxr::UsdShadeInput In = ConvertShader.CreateInput(pxr::TfToken("in"), InType);
		if (IsColorType(InType))
		{
			In.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
		}

		const int32 SrcDim = GetChannelCount(InType);
		const int32 DstDim = GetChannelCount(OutType);

		if (InType == pxr::SdfValueTypeNames->Float)
		{
			FString Channels;
			if (OutType == pxr::SdfValueTypeNames->Color3f) Channels = TEXT("rrr");
			else if (OutType == pxr::SdfValueTypeNames->Color4f) Channels = TEXT("rrrr");
			else if (OutType == pxr::SdfValueTypeNames->Float2) Channels = TEXT("xx");
			else if (OutType == pxr::SdfValueTypeNames->Float3) Channels = TEXT("xxx");
			else if (OutType == pxr::SdfValueTypeNames->Float4) Channels = TEXT("xxxx");
			else if (OutType == pxr::SdfValueTypeNames->Float) Channels = TEXT("x");
			else return false;

			ConvertShader.CreateInput(pxr::TfToken("channels"), pxr::SdfValueTypeNames->String).Set(std::string(TCHAR_TO_UTF8(*Channels)));
			ConvertShader.CreateOutput(pxr::TfToken("out"), OutType);
			ConversionType = FString::Printf(TEXT("swizzle_%s_%s"), *InMx, *OutMx);
			return true;
		}

		FString SrcOrder;
		if (InType == pxr::SdfValueTypeNames->Color3f) SrcOrder = TEXT("rgb");
		else if (InType == pxr::SdfValueTypeNames->Color4f) SrcOrder = TEXT("rgba");
		else if (InType == pxr::SdfValueTypeNames->Float2) SrcOrder = TEXT("xy");
		else if (InType == pxr::SdfValueTypeNames->Float3) SrcOrder = TEXT("xyz");
		else if (InType == pxr::SdfValueTypeNames->Float4) SrcOrder = TEXT("xyzw");

		if (SrcOrder.IsEmpty() || SrcDim <= 0 || DstDim <= 0)
		{
			return false;
		}

		FString Channels;
		if (DstDim <= SrcDim)
		{
			Channels = SrcOrder.Left(DstDim);
		}
		else
		{
			Channels = SrcOrder;
			const int32 Extra = DstDim - SrcDim;
			for (int32 I = 0; I < Extra; ++I)
			{
				const int32 TargetIndex = SrcDim + I;
				const bool bIsLastChannel = (TargetIndex == 3);
				if (bIsLastChannel && (OutType == pxr::SdfValueTypeNames->Float4 || OutType == pxr::SdfValueTypeNames->Color4f))
				{
					Channels.AppendChar(TEXT('1'));
				}
				else
				{
					Channels.AppendChar(TEXT('0'));
				}
			}
		}

		ConvertShader.CreateInput(pxr::TfToken("channels"), pxr::SdfValueTypeNames->String).Set(std::string(TCHAR_TO_UTF8(*Channels)));
		ConvertShader.CreateOutput(pxr::TfToken("out"), OutType);
		ConversionType = FString::Printf(TEXT("swizzle_%s_%s"), *InMx, *OutMx);
		return true;
	};

	if (FromType == pxr::SdfValueTypeNames->Float && ToType == pxr::SdfValueTypeNames->Color3f)
	{
		CreateConvert(TEXT("ND_convert_float_color3"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Float && ToType == pxr::SdfValueTypeNames->Color4f)
	{
		CreateConvert(TEXT("ND_convert_float_color4"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Float && ToType == pxr::SdfValueTypeNames->Float2)
	{
		CreateConvert(TEXT("ND_convert_float_vector2"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Float && ToType == pxr::SdfValueTypeNames->Float3)
	{
		CreateConvert(TEXT("ND_convert_float_vector3"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Float && ToType == pxr::SdfValueTypeNames->Float4)
	{
		CreateConvert(TEXT("ND_convert_float_vector4"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Float2 && ToType == pxr::SdfValueTypeNames->Float3)
	{
		CreateConvert(TEXT("ND_convert_vector2_vector3"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Float3 && ToType == pxr::SdfValueTypeNames->Color3f)
	{
		CreateConvert(TEXT("ND_convert_vector3_color3"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Float3 && ToType == pxr::SdfValueTypeNames->Float2)
	{
		CreateConvert(TEXT("ND_convert_vector3_vector2"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Float3 && ToType == pxr::SdfValueTypeNames->Float4)
	{
		CreateConvert(TEXT("ND_convert_vector3_vector4"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Float4 && ToType == pxr::SdfValueTypeNames->Color4f)
	{
		CreateConvert(TEXT("ND_convert_vector4_color4"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Float4 && ToType == pxr::SdfValueTypeNames->Float3)
	{
		CreateConvert(TEXT("ND_convert_vector4_vector3"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Color3f && ToType == pxr::SdfValueTypeNames->Float3)
	{
		CreateConvert(TEXT("ND_convert_color3_vector3"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Color4f && ToType == pxr::SdfValueTypeNames->Float4)
	{
		CreateConvert(TEXT("ND_convert_color4_vector4"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Color3f && ToType == pxr::SdfValueTypeNames->Color4f)
	{
		CreateConvert(TEXT("ND_convert_color3_color4"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Color4f && ToType == pxr::SdfValueTypeNames->Color3f)
	{
		CreateConvert(TEXT("ND_convert_color4_color3"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Bool && ToType == pxr::SdfValueTypeNames->Float)
	{
		CreateConvert(TEXT("ND_convert_boolean_float"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Int && ToType == pxr::SdfValueTypeNames->Float)
	{
		CreateConvert(TEXT("ND_convert_integer_float"), FromType, ToType);
	}
	else if (FromType == pxr::SdfValueTypeNames->Float && ToType == pxr::SdfValueTypeNames->Int)
	{
		CreateConvert(TEXT("ND_convert_float_integer"), FromType, ToType);
	}
	else
	{
		const FString InMx = GetMxTypeName(FromType);
		const FString OutMx = GetMxTypeName(ToType);
		const FString SwizzleId = FString::Printf(TEXT("ND_swizzle_%s_%s"), *InMx, *OutMx);

		if (!CreateSwizzle(SwizzleId, FromType, ToType))
		{
		SetFailure(TEXT("TypeConvert"), FString::Printf(TEXT("Unsupported type conversion %s -> %s"),
			UTF8_TO_TCHAR(FromType.GetAsToken().GetString().c_str()),
			UTF8_TO_TCHAR(ToType.GetAsToken().GetString().c_str())));
		UE_LOG(LogPICOSpatialUSD, Error,
			TEXT("%s[FAIL][TypeConvert] Unsupported type conversion from %s to %s"),
			*GetLogPrefix(),
				UTF8_TO_TCHAR(FromType.GetAsToken().GetString().c_str()),
				UTF8_TO_TCHAR(ToType.GetAsToken().GetString().c_str()));
			return pxr::UsdShadeShader();
		}
	}
	
	UE_LOG(LogPICOSpatialUSD, Verbose,
		TEXT("%s[CONVERT] %s -> %s (%s) %s"),
		*GetLogPrefix(),
		UTF8_TO_TCHAR(FromType.GetAsToken().GetString().c_str()),
		UTF8_TO_TCHAR(ToType.GetAsToken().GetString().c_str()),
		*ConversionType, *ConvertShaderPath);
	
	return ConvertShader;
}

// ============================================================================
// Helper Functions Implementation
// ============================================================================

const UMaterial* FShaderGraphUsdConvertorPICO::ValidateAndCastMaterial() const
{
	if (!MaterialInterface)
	{
		const_cast<FShaderGraphUsdConvertorPICO*>(this)->SetFailure(TEXT("Validate"), TEXT("MaterialInterface is null"));
		return nullptr;
	}

	const UMaterial* Material = Cast<UMaterial>(MaterialInterface);
	if (!Material)
	{
		const_cast<FShaderGraphUsdConvertorPICO*>(this)->SetFailure(TEXT("Validate"), FString::Printf(TEXT("%s is not a UMaterial, only UMaterial is supported"), *MaterialInterface->GetName()));
		return nullptr;
	}

	if (!Stage)
	{
		const_cast<FShaderGraphUsdConvertorPICO*>(this)->SetFailure(TEXT("Validate"), TEXT("USD Stage is null"));
		return nullptr;
	}

	if (Material->bUseMaterialAttributes)
	{
		const_cast<FShaderGraphUsdConvertorPICO*>(this)->SetFailure(TEXT("Validate"), TEXT("Use Material Attributes is not supported by USD shader graph export"));
		return nullptr;
	}

	return Material;
}

bool FShaderGraphUsdConvertorPICO::ValidateShadingModel(const UMaterial* Material, EMaterialShadingModel& OutShadingModel) const
{
	check(Material);
	
	FMaterialShadingModelField ShadingModels = Material->GetShadingModels();
	
	if (!ShadingModels.IsValid() || ShadingModels.CountShadingModels() != 1)
	{
		const_cast<FShaderGraphUsdConvertorPICO*>(this)->SetFailure(TEXT("ValidateShadingModel"),
			FString::Printf(TEXT("Multiple or invalid shading models (count=%d)"), ShadingModels.CountShadingModels()));
		return false;
	}

	OutShadingModel = ShadingModels.GetFirstShadingModel();
	
	// Validate that the shading model is supported in CreateSurfaceShader
	if (OutShadingModel != EMaterialShadingModel::MSM_DefaultLit &&
		OutShadingModel != EMaterialShadingModel::MSM_ClearCoat &&
		OutShadingModel != EMaterialShadingModel::MSM_Unlit)
	{
		const_cast<FShaderGraphUsdConvertorPICO*>(this)->SetFailure(TEXT("ValidateShadingModel"),
			FString::Printf(TEXT("Unsupported shading model=%d (only DefaultLit/ClearCoat/Unlit)"), (int)OutShadingModel));
		return false;
	}
	
	return true;
}

// ============================================================================
// USD Setup Functions
// ============================================================================

void FShaderGraphUsdConvertorPICO::SetupUsdStageDefaults()
{
	check(Stage);
	
	Stage->SetDefaultPrim(Stage->DefinePrim(pxr::SdfPath("/Root")));
	Stage->SetMetadata(pxr::TfToken("metersPerUnit"), 1.0);
	Stage->SetMetadata(pxr::TfToken("upAxis"), pxr::TfToken("Y"));
}

void FShaderGraphUsdConvertorPICO::CreatePreviewGeometry()
{
	check(Stage);
	
	// Define a cube geometry for material preview
	pxr::UsdGeomCube UsdCube = pxr::UsdGeomCube::Define(Stage, pxr::SdfPath("/Root/Cube"));
	UsdCube.GetSizeAttr().Set(0.5);

	// Apply MaterialBindingAPI properly
	pxr::UsdShadeMaterialBindingAPI BindingAPI = pxr::UsdShadeMaterialBindingAPI::Apply(UsdCube.GetPrim());

	// Bind material to cube
	BindingAPI.Bind(pxr::UsdShadeMaterial::Get(Stage, pxr::SdfPath("/Root/Material")));
}

bool FShaderGraphUsdConvertorPICO::CreateUsdMaterialWithProperties(const UMaterial* Material, pxr::UsdShadeMaterial& UsdMaterial)
{
	check(Stage);
	check(Material);
	
	// Convert blend mode
	EBlendMode BlendMode = Material->GetBlendMode();
	bool ApplyToneMapping = false;
	std::string UsdBlendingMode;
   std::string UsdCullingMode = Material->IsTwoSided() ? "NONE" : "BACK";
	std::string MaterialType = "SHADER_GRAPH";
	float OpacityThreshold = 0.0f;
	
	switch (BlendMode)
	{
	case BLEND_Opaque:
		UsdBlendingMode = "OPAQUE";
		break;
	case BLEND_Masked:
		UsdBlendingMode = "MASKED";
		OpacityThreshold = Material->GetOpacityMaskClipValue();
		break;
	case BLEND_Translucent:
	case BLEND_AlphaComposite:
	case BLEND_AlphaHoldout:
		UsdBlendingMode = "TRANSPARENT";
		break;
	case BLEND_Additive:
		UsdBlendingMode = "ADD";
		break;
	default:
		UE_LOG(LogPICOSpatialUSD, Warning, 
			TEXT("Unsupported blend mode %d for material:%s."), 
			(int)BlendMode, *Material->GetName());
		return false;
		break;
	}

	UsdMaterial = pxr::UsdShadeMaterial::Define(Stage, pxr::SdfPath("/Root/Material"));
	pxr::UsdPrim MaterialPrim = UsdMaterial.GetPrim();
	MaterialPrim.SetCustomDataByKey(pxr::TfToken("applyToneMapping"), pxr::VtValue(ApplyToneMapping));
	MaterialPrim.SetCustomDataByKey(pxr::TfToken("blendingMode"), pxr::VtValue(UsdBlendingMode));
   MaterialPrim.SetCustomDataByKey(pxr::TfToken("cullingMode"), pxr::VtValue(UsdCullingMode));
	MaterialPrim.SetCustomDataByKey(pxr::TfToken("materialType"), pxr::VtValue(MaterialType));
	MaterialPrim.SetCustomDataByKey(pxr::TfToken("opacityThreshold"), pxr::VtValue(OpacityThreshold));
	
	return true;
}

bool FShaderGraphUsdConvertorPICO::CreateSurfaceShader(const UMaterial* Material, EMaterialShadingModel ShadingModel,
	pxr::UsdShadeMaterial& UsdMaterial, pxr::UsdShadeShader& OutSurfaceShader)
{
	check(Stage);
	check(Material);
	const bool bMaskedMaterial = Material->GetBlendMode() == BLEND_Masked;
	const float OpacityThreshold = bMaskedMaterial ? Material->GetOpacityMaskClipValue() : 0.0f;
	
	// Create material surface output
	pxr::UsdShadeOutput Surface = UsdMaterial.CreateOutput(pxr::TfToken("mtlx:surface"), pxr::SdfValueTypeNames->Token);
	
	if (ShadingModel == EMaterialShadingModel::MSM_DefaultLit || ShadingModel == EMaterialShadingModel::MSM_ClearCoat)
	{
		// Create UsdPreviewSurface shader
		OutSurfaceShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath("/Root/Material/Surface"));
		OutSurfaceShader.CreateIdAttr().Set(pxr::TfToken("ND_UsdPreviewSurface_surfaceshader"));
		
		// Define inputs with default values
		OutSurfaceShader.CreateInput(pxr::TfToken("clearcoat"), pxr::SdfValueTypeNames->Float).Set(0.0f);
		OutSurfaceShader.CreateInput(pxr::TfToken("clearcoatRoughness"), pxr::SdfValueTypeNames->Float).Set(0.01f);
		{
			pxr::UsdShadeInput DiffuseColor = OutSurfaceShader.CreateInput(pxr::TfToken("diffuseColor"), pxr::SdfValueTypeNames->Color3f);
			DiffuseColor.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
			DiffuseColor.Set(pxr::GfVec3f(0.18f, 0.18f, 0.18f));
		}
		OutSurfaceShader.CreateInput(pxr::TfToken("displacement"), pxr::SdfValueTypeNames->Float).Set(0.0f);
		{
			pxr::UsdShadeInput EmissiveColor = OutSurfaceShader.CreateInput(pxr::TfToken("emissiveColor"), pxr::SdfValueTypeNames->Color3f);
			EmissiveColor.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
			EmissiveColor.Set(pxr::GfVec3f(0.0f, 0.0f, 0.0f));
		}
		OutSurfaceShader.CreateInput(pxr::TfToken("ior"), pxr::SdfValueTypeNames->Float).Set(1.5f);
		OutSurfaceShader.CreateInput(pxr::TfToken("metallic"), pxr::SdfValueTypeNames->Float).Set(0.0f);
		OutSurfaceShader.CreateInput(pxr::TfToken("normal"), pxr::SdfValueTypeNames->Float3).Set(pxr::GfVec3f(0.0f, 0.0f, 1.0f));
		OutSurfaceShader.CreateInput(pxr::TfToken("occlusion"), pxr::SdfValueTypeNames->Float).Set(1.0f);
		OutSurfaceShader.CreateInput(pxr::TfToken("opacity"), pxr::SdfValueTypeNames->Float).Set(1.0f);
		OutSurfaceShader.CreateInput(pxr::TfToken("opacityThreshold"), pxr::SdfValueTypeNames->Float).Set(OpacityThreshold);
		OutSurfaceShader.CreateInput(pxr::TfToken("roughness"), pxr::SdfValueTypeNames->Float).Set(0.5f);
		{
			pxr::UsdShadeInput SpecularColor = OutSurfaceShader.CreateInput(pxr::TfToken("specularColor"), pxr::SdfValueTypeNames->Color3f);
			SpecularColor.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
			SpecularColor.Set(pxr::GfVec3f(0.0f, 0.0f, 0.0f));
		}
		OutSurfaceShader.CreateInput(pxr::TfToken("useSpecularWorkflow"), pxr::SdfValueTypeNames->Int).Set(0);
	}
	else if (ShadingModel == EMaterialShadingModel::MSM_Unlit)
	{
		// Create RealityKit unlit shader
		OutSurfaceShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath("/Root/Material/Surface"));
		OutSurfaceShader.CreateIdAttr().Set(pxr::TfToken("ND_realitykit_unlit_surfaceshader"));
		
		// Define inputs with default values
		OutSurfaceShader.CreateInput(pxr::TfToken("applyPostProcessToneMap"), pxr::SdfValueTypeNames->Bool).Set(true);
		{
			pxr::UsdShadeInput Color = OutSurfaceShader.CreateInput(pxr::TfToken("color"), pxr::SdfValueTypeNames->Color3f);
			Color.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
			Color.Set(pxr::GfVec3f(1.0f, 1.0f, 1.0f));
		}
		OutSurfaceShader.CreateInput(pxr::TfToken("hasPremultipliedAlpha"), pxr::SdfValueTypeNames->Bool).Set(false);
		OutSurfaceShader.CreateInput(pxr::TfToken("opacity"), pxr::SdfValueTypeNames->Float).Set(1.0f);
		OutSurfaceShader.CreateInput(pxr::TfToken("opacityThreshold"), pxr::SdfValueTypeNames->Float).Set(OpacityThreshold);
	}
	else
	{
		SetFailure(TEXT("CreateSurfaceShader"), FString::Printf(TEXT("Unsupported shading model=%d"), (int)ShadingModel));
		return false;
	}
	
	// Create and connect surface output
	pxr::UsdShadeOutput SurfaceOutput = OutSurfaceShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Token);
	Surface.ConnectToSource(SurfaceOutput);
	
	return true;
}

bool FShaderGraphUsdConvertorPICO::ProcessMaterialExpressions(const UMaterial* Material)
{
	check(Material);

	bHasUnsupportedNode = false;
	if (!MaterialExpressionToPICONodeMap.IsEmpty())
	{
		for (auto& Item : MaterialExpressionToPICONodeMap)
		{
			delete Item.Value;
		}
		MaterialExpressionToPICONodeMap.Empty();
	}
	
	// Step 1: Create PICO node wrappers and validate node support
	CreateAndValidateNodes(Material);
	if (bHasUnsupportedNode)
	{
		SetFailure(TEXT("ProcessMaterialExpressions"), LastError.IsEmpty() ? TEXT("Unsupported expression node detected") : LastError);
		return false;
	}

	// Step 1.5: Infer USD types via forward propagation so that DefineShader()
	// implementations can emit the correct type-specific nodedef (e.g.
	// ND_multiply_color3 instead of always ND_multiply_color4).
	RunForwardTypePropagation();

	// Step 2: Define USD shaders for all nodes
	if (!DefineAllUsdShaders())
	{
		SetFailure(TEXT("DefineAllUsdShaders"), LastError.IsEmpty() ? TEXT("Failed to define all nodes") : LastError);
		return false;
	}
	
	// Step 3: Connect node inputs
	if (!ConnectAllNodeInputs())
	{
		SetFailure(TEXT("ConnectAllNodeInputs"), LastError.IsEmpty() ? TEXT("Failed to connect node inputs") : LastError);
		return false;
	}
	
	return true;
}

void FShaderGraphUsdConvertorPICO::CreateAndValidateNodes(const UMaterial* Material)
{
	check(Material);

	const UMaterialEditorOnlyData* EditorOnly = Material->GetEditorOnlyData();
	if (!EditorOnly)
	{
		SetFailure(TEXT("ValidateExpressions"), TEXT("Material editor data not available"));
		bHasUnsupportedNode = true;
		return;
	}

	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	const bool bExportUnconnectedNodes = Settings && Settings->bExportUnconnectedUsdShaderGraphNodes;
	TSet<UMaterialExpression*> ExpressionsToExport;
	ReachableMaterialExpressions.Reset();
	TFunction<void(UMaterialExpression*)> AddReachableExpression = [&](UMaterialExpression* Expression)
	{
		if (!Expression || ExpressionsToExport.Contains(Expression))
		{
			return;
		}

		ExpressionsToExport.Add(Expression);
		ReachableMaterialExpressions.Add(Expression);
		const int32 InputCount = Expression->CountInputs();
		for (int32 InputIndex = 0; InputIndex < InputCount; ++InputIndex)
		{
			const FExpressionInput* Input = Expression->GetInput(InputIndex);
			if (Input && Input->IsConnected() && Input->Expression)
			{
				AddReachableExpression(Input->Expression);
			}
		}
	};

   auto AddReachableFromMaterialProperty = [&](EMaterialProperty Property, const FExpressionInput& MaterialInput)
	{
           if (!IsMaterialPropertyActiveForExport(Material, Property))
           {
                   return;
           }

		if (MaterialInput.IsConnected() && MaterialInput.Expression)
		{
			AddReachableExpression(MaterialInput.Expression);
		}
	};

   AddReachableFromMaterialProperty(MP_BaseColor, EditorOnly->BaseColor);
   AddReachableFromMaterialProperty(MP_Metallic, EditorOnly->Metallic);
   AddReachableFromMaterialProperty(MP_Specular, EditorOnly->Specular);
   AddReachableFromMaterialProperty(MP_Roughness, EditorOnly->Roughness);
   AddReachableFromMaterialProperty(MP_EmissiveColor, EditorOnly->EmissiveColor);
   AddReachableFromMaterialProperty(MP_Normal, EditorOnly->Normal);
   AddReachableFromMaterialProperty(MP_Opacity, EditorOnly->Opacity);
   AddReachableFromMaterialProperty(MP_OpacityMask, EditorOnly->OpacityMask);
   AddReachableFromMaterialProperty(MP_AmbientOcclusion, EditorOnly->AmbientOcclusion);
   AddReachableFromMaterialProperty(MP_CustomData0, EditorOnly->ClearCoat);
   AddReachableFromMaterialProperty(MP_CustomData1, EditorOnly->ClearCoatRoughness);
   AddReachableFromMaterialProperty(MP_Refraction, EditorOnly->Refraction);
   AddReachableFromMaterialProperty(MP_Displacement, EditorOnly->Displacement);

	if (bExportUnconnectedNodes)
	{
		for (UMaterialExpression* Expression : Material->GetExpressions())
		{
			if (Expression)
			{
				ExpressionsToExport.Add(Expression);
			}
		}
	}

	for (UMaterialExpression* Expression : ExpressionsToExport)
	{
		// Create or find PICO node wrapper
		if (FExpressionBasePICO** ExistingNode = MaterialExpressionToPICONodeMap.Find(Expression))
		{
			if (!(*ExistingNode) && ReachableMaterialExpressions.Contains(Expression))
			{
				bHasUnsupportedNode = true;
				return;
			}
			continue;
		}

		FExpressionBasePICO* Node = CreateExpressionPICO(Expression);
		if (!Node)
		{
			const FString UnsupportedReason = FString::Printf(TEXT("Unsupported expression '%s' (%s)"), *Expression->GetName(), *Expression->GetClass()->GetName());
			if (ReachableMaterialExpressions.Contains(Expression))
			{
				SetFailure(TEXT("UnsupportedNode"), UnsupportedReason);
				bHasUnsupportedNode = true;
				return;
			}

			UE_LOG(LogPICOSpatialUSD, Verbose,
				TEXT("%s[SKIP][UNCONNECTED] %s"),
				*GetLogPrefix(),
				*UnsupportedReason);
			continue;
		}

		MaterialExpressionToPICONodeMap.Add(Expression, Node);
		
	}
}

bool FShaderGraphUsdConvertorPICO::DefineAllUsdShaders()
{
	for (auto& Item : MaterialExpressionToPICONodeMap)
	{
		FExpressionBasePICO* Node = Item.Value;
		const bool bFallbackCriticalNode = ReachableMaterialExpressions.Contains(Item.Key);
		if (!Node)
		{
			if (bFallbackCriticalNode)
			{
				return false;
			}
			continue;
		}

		Node->DefineShader(Stage);
		if (!Node->bDefinedUsdShader)
		{
			const FString FailureReason = FString::Printf(TEXT("DefineShader failed for node '%s' (Type=%d)"), *Node->NodeName, (int)Node->NodeType);
			if (bFallbackCriticalNode)
			{
				SetFailure(TEXT("DefineShader"), FailureReason);
				return false;
			}

			UE_LOG(LogPICOSpatialUSD, Warning,
				TEXT("%s[SKIP][UNCONNECTED] %s"),
				*GetLogPrefix(),
				*FailureReason);
		}
	}
	return true;
}

bool FShaderGraphUsdConvertorPICO::ConnectAllNodeInputs()
{
	for (auto& Item : MaterialExpressionToPICONodeMap)
	{
		FExpressionBasePICO* Node = Item.Value;
		const bool bFallbackCriticalNode = ReachableMaterialExpressions.Contains(Item.Key);
		if (!Node)
		{
			if (bFallbackCriticalNode)
			{
				return false;
			}
			continue;
		}

		if (!Node->bDefinedUsdShader && !bFallbackCriticalNode)
		{
			continue;
		}

		if (!Node->ConnectInput())
		{
			const FString FailureReason = FString::Printf(TEXT("ConnectInput failed for node '%s' (Type=%d)"), *Node->NodeName, (int)Node->NodeType);
			if (bFallbackCriticalNode)
			{
				SetFailure(TEXT("ConnectInput"), FailureReason);
				return false;
			}

			UE_LOG(LogPICOSpatialUSD, Warning,
				TEXT("%s[SKIP][UNCONNECTED] %s"),
				*GetLogPrefix(),
				*FailureReason);
		}
	}
	return true;
}

bool FShaderGraphUsdConvertorPICO::ConnectMaterialPropertiesToSurface(const UMaterial* Material, 
	EMaterialShadingModel ShadingModel, pxr::UsdShadeShader& SurfaceShader)
{
	const UMaterialEditorOnlyData* EditorOnly = Material->GetEditorOnlyData();
	if (!EditorOnly)
	{
		SetFailure(TEXT("ConnectMaterialProperties"), TEXT("Material editor data not available"));
		return false;
	}
	
	bool bAllConnectionsSucceeded = true;
	
	// Helper lambda to connect material input to surface shader with type conversion
	auto ConnectMaterialInput = [&](const FExpressionInput* MaterialInput, const pxr::TfToken& ShaderInputName) -> bool
		{
			if (!MaterialInput)
			{
				return false;
			}

			if (MaterialInput->IsConnected() && MaterialInput->Expression)
			{
				if (FExpressionBasePICO* ConnectedNode = FindNode(MaterialInput->Expression))
				{
					if (ConnectedNode->bDefinedUsdShader && ConnectedNode->ShaderOutputs.Contains(MaterialInput->OutputIndex))
					{
						pxr::UsdShadeInput ShaderInput = SurfaceShader.GetInput(ShaderInputName);
						if (!ShaderInput)
						{
							SetFailure(TEXT("ConnectMaterialProperties"),
								FString::Printf(TEXT("Missing surface shader input '%s'"), *FString(ShaderInputName.GetString().c_str())));
							return false;
						}

						// Get output type from connected node
						pxr::SdfValueTypeName OutputType = ConnectedNode->GetOutputType(MaterialInput->OutputIndex);
						pxr::SdfValueTypeName ExpectedType = ShaderInput.GetTypeName();

						// Check if type conversion is needed
						if (OutputType && ExpectedType && OutputType != ExpectedType)
						{
							// Create conversion node
							pxr::UsdShadeShader ConvertShader = CreateConvertOrSwizzleShader(OutputType, ExpectedType);

							if (ConvertShader)
							{
								pxr::UsdShadeInput ConvertInput = ConvertShader.GetInput(pxr::TfToken("in"));
								pxr::UsdShadeOutput ConvertOutput = ConvertShader.GetOutput(pxr::TfToken("out"));

								if (ConvertInput && ConvertOutput)
								{
									// Connect source node -> convert node
									if (!ConvertInput.ConnectToSource(ConnectedNode->ShaderOutputs[MaterialInput->OutputIndex]))
									{
										SetFailure(TEXT("ConnectMaterialProperties"),
											FString::Printf(TEXT("Failed to connect '%s' to convert input for '%s'"), *ConnectedNode->NodeName, *FString(ShaderInputName.GetString().c_str())));
										return false;
									}
									// Connect convert node -> surface shader
									if (!ShaderInput.ConnectToSource(ConvertOutput))
									{
										SetFailure(TEXT("ConnectMaterialProperties"),
											FString::Printf(TEXT("Failed to connect convert output to '%s'"), *FString(ShaderInputName.GetString().c_str())));
										return false;
									}

									UE_LOG(LogPICOSpatialUSD, Verbose, TEXT("%s[CONNECT] %s -> %s (convert %s -> %s)"),
										*GetLogPrefix(),
										*ConnectedNode->NodeName, *FString(ShaderInputName.GetString().c_str()), 
										UTF8_TO_TCHAR(OutputType.GetAsToken().GetString().c_str()), 
										UTF8_TO_TCHAR(ExpectedType.GetAsToken().GetString().c_str()));
								}
								else
								{
									SetFailure(TEXT("ConnectMaterialProperties"),
										FString::Printf(TEXT("Missing convert pins for '%s'"), *FString(ShaderInputName.GetString().c_str())));
									return false;
								}
							}
							else
							{
								SetFailure(TEXT("ConnectMaterialProperties"),
									FString::Printf(TEXT("Failed to create convert node for '%s' (%s -> %s)"),
										*FString(ShaderInputName.GetString().c_str()),
										UTF8_TO_TCHAR(OutputType.GetAsToken().GetString().c_str()),
										UTF8_TO_TCHAR(ExpectedType.GetAsToken().GetString().c_str())));
								return false;
							}
						}
						else
						{
							// Direct connection without conversion
							if (!ShaderInput.ConnectToSource(ConnectedNode->ShaderOutputs[MaterialInput->OutputIndex]))
							{
								SetFailure(TEXT("ConnectMaterialProperties"),
									FString::Printf(TEXT("Failed to connect '%s' to '%s'"), *ConnectedNode->NodeName, *FString(ShaderInputName.GetString().c_str())));
								return false;
							}
							UE_LOG(LogPICOSpatialUSD, Verbose, TEXT("%s[CONNECT] %s -> %s"),
								*GetLogPrefix(), *ConnectedNode->NodeName, *FString(ShaderInputName.GetString().c_str()));
						}
					}
					else
					{
						SetFailure(TEXT("ConnectMaterialProperties"),
							FString::Printf(TEXT("Missing upstream output %d for node '%s'"), MaterialInput->OutputIndex, *ConnectedNode->NodeName));
						return false;
					}
				}
				else
				{
					SetFailure(TEXT("ConnectMaterialProperties"), TEXT("Missing upstream node"));
					return false;
				}
			}
			return true;
		};

   auto GetSurfaceShaderInput = [&](const pxr::TfToken& ShaderInputName) -> pxr::UsdShadeInput
           {
                   pxr::UsdShadeInput ShaderInput = SurfaceShader.GetInput(ShaderInputName);
                   if (!ShaderInput)
                   {
                           SetFailure(TEXT("ConnectMaterialProperties"),
                                   FString::Printf(TEXT("Missing surface shader input '%s'"), *FString(ShaderInputName.GetString().c_str())));
                   }
                   return ShaderInput;
           };

   auto SetSurfaceShaderInputFromScalarConstant = [&](const pxr::TfToken& ShaderInputName, float ConstantValue) -> bool
           {
                   pxr::UsdShadeInput ShaderInput = GetSurfaceShaderInput(ShaderInputName);
                   if (!ShaderInput)
                   {
                           return false;
                   }

                   const pxr::SdfValueTypeName ShaderInputType = ShaderInput.GetTypeName();
                   if (ShaderInputType == pxr::SdfValueTypeNames->Float)
                   {
                           ShaderInput.Set(ConstantValue);
                   }
                   else if (ShaderInputType == pxr::SdfValueTypeNames->Color3f)
                   {
                           ShaderInput.Set(pxr::GfVec3f(ConstantValue, ConstantValue, ConstantValue));
                   }
                   else if (ShaderInputType == pxr::SdfValueTypeNames->Color4f)
                   {
                           ShaderInput.Set(pxr::GfVec4f(ConstantValue, ConstantValue, ConstantValue, ConstantValue));
                   }
                   else if (ShaderInputType == pxr::SdfValueTypeNames->Float2)
                   {
                           ShaderInput.Set(pxr::GfVec2f(ConstantValue, ConstantValue));
                   }
                   else if (ShaderInputType == pxr::SdfValueTypeNames->Float3)
                   {
                           ShaderInput.Set(pxr::GfVec3f(ConstantValue, ConstantValue, ConstantValue));
                   }
                   else if (ShaderInputType == pxr::SdfValueTypeNames->Float4)
                   {
                           ShaderInput.Set(pxr::GfVec4f(ConstantValue, ConstantValue, ConstantValue, ConstantValue));
                   }
                   else
                   {
                           SetFailure(TEXT("ConnectMaterialProperties"),
                                   FString::Printf(TEXT("Unsupported surface input type '%s' for scalar constant on '%s'"),
                                           UTF8_TO_TCHAR(ShaderInputType.GetAsToken().GetString().c_str()),
                                           *FString(ShaderInputName.GetString().c_str())));
                           return false;
                   }

                   UE_LOG(LogPICOSpatialUSD, Verbose, TEXT("%s[CONST] %s = %f"),
                           *GetLogPrefix(), *FString(ShaderInputName.GetString().c_str()), ConstantValue);
                   return true;
           };

   auto SetSurfaceShaderInputFromColorConstant = [&](const pxr::TfToken& ShaderInputName, const FLinearColor& ConstantValue) -> bool
           {
                   pxr::UsdShadeInput ShaderInput = GetSurfaceShaderInput(ShaderInputName);
                   if (!ShaderInput)
                   {
                           return false;
                   }

                   const pxr::SdfValueTypeName ShaderInputType = ShaderInput.GetTypeName();
                   if (ShaderInputType == pxr::SdfValueTypeNames->Float)
                   {
                           ShaderInput.Set(ConstantValue.R);
                   }
                   else if (ShaderInputType == pxr::SdfValueTypeNames->Color3f)
                   {
                           ShaderInput.Set(pxr::GfVec3f(ConstantValue.R, ConstantValue.G, ConstantValue.B));
                   }
                   else if (ShaderInputType == pxr::SdfValueTypeNames->Color4f)
                   {
                           ShaderInput.Set(pxr::GfVec4f(ConstantValue.R, ConstantValue.G, ConstantValue.B, ConstantValue.A));
                   }
                   else if (ShaderInputType == pxr::SdfValueTypeNames->Float3)
                   {
                           ShaderInput.Set(pxr::GfVec3f(ConstantValue.R, ConstantValue.G, ConstantValue.B));
                   }
                   else if (ShaderInputType == pxr::SdfValueTypeNames->Float4)
                   {
                           ShaderInput.Set(pxr::GfVec4f(ConstantValue.R, ConstantValue.G, ConstantValue.B, ConstantValue.A));
                   }
                   else
                   {
                           SetFailure(TEXT("ConnectMaterialProperties"),
                                   FString::Printf(TEXT("Unsupported surface input type '%s' for color constant on '%s'"),
                                           UTF8_TO_TCHAR(ShaderInputType.GetAsToken().GetString().c_str()),
                                           *FString(ShaderInputName.GetString().c_str())));
                           return false;
                   }

                   UE_LOG(LogPICOSpatialUSD, Verbose, TEXT("%s[CONST] %s = (%f, %f, %f, %f)"),
                           *GetLogPrefix(), *FString(ShaderInputName.GetString().c_str()),
                           ConstantValue.R, ConstantValue.G, ConstantValue.B, ConstantValue.A);
                   return true;
           };

   auto SetSurfaceShaderInputFromVectorConstant = [&](const pxr::TfToken& ShaderInputName, const FVector3f& ConstantValue) -> bool
           {
                   pxr::UsdShadeInput ShaderInput = GetSurfaceShaderInput(ShaderInputName);
                   if (!ShaderInput)
                   {
                           return false;
                   }

                   const pxr::SdfValueTypeName ShaderInputType = ShaderInput.GetTypeName();
                   if (ShaderInputType == pxr::SdfValueTypeNames->Float)
                   {
                           ShaderInput.Set(ConstantValue.X);
                   }
                   else if (ShaderInputType == pxr::SdfValueTypeNames->Color3f)
                   {
                           ShaderInput.Set(pxr::GfVec3f(ConstantValue.X, ConstantValue.Y, ConstantValue.Z));
                   }
                   else if (ShaderInputType == pxr::SdfValueTypeNames->Float3)
                   {
                           ShaderInput.Set(pxr::GfVec3f(ConstantValue.X, ConstantValue.Y, ConstantValue.Z));
                   }
                   else
                   {
                           SetFailure(TEXT("ConnectMaterialProperties"),
                                   FString::Printf(TEXT("Unsupported surface input type '%s' for vector constant on '%s'"),
                                           UTF8_TO_TCHAR(ShaderInputType.GetAsToken().GetString().c_str()),
                                           *FString(ShaderInputName.GetString().c_str())));
                           return false;
                   }

                   UE_LOG(LogPICOSpatialUSD, Verbose, TEXT("%s[CONST] %s = (%f, %f, %f)"),
                           *GetLogPrefix(), *FString(ShaderInputName.GetString().c_str()),
                           ConstantValue.X, ConstantValue.Y, ConstantValue.Z);
                   return true;
           };

   auto ConnectScalarMaterialInput = [&](EMaterialProperty Property, const FScalarMaterialInput& MaterialInput, const pxr::TfToken& ShaderInputName,
           const TOptional<float>& FallbackValue = TOptional<float>()) -> bool
           {
                   if (!IsMaterialPropertyActiveForExport(Material, Property))
                   {
                           if (FallbackValue.IsSet())
                           {
                                   return SetSurfaceShaderInputFromScalarConstant(ShaderInputName, FallbackValue.GetValue());
                           }
                           return true;
                   }

                   if (MaterialInput.IsConnected() && MaterialInput.Expression)
                   {
                           return ConnectMaterialInput(&MaterialInput, ShaderInputName);
                   }

                   if (MaterialInput.UseConstant)
                   {
                           return SetSurfaceShaderInputFromScalarConstant(ShaderInputName, MaterialInput.Constant);
                   }

                   if (FallbackValue.IsSet())
                   {
                           return SetSurfaceShaderInputFromScalarConstant(ShaderInputName, FallbackValue.GetValue());
                   }

                   return true;
           };

   auto ConnectColorMaterialInput = [&](EMaterialProperty Property, const FColorMaterialInput& MaterialInput, const pxr::TfToken& ShaderInputName,
           const TOptional<FLinearColor>& FallbackValue = TOptional<FLinearColor>()) -> bool
           {
                   if (!IsMaterialPropertyActiveForExport(Material, Property))
                   {
                           if (FallbackValue.IsSet())
                           {
                                   return SetSurfaceShaderInputFromColorConstant(ShaderInputName, FallbackValue.GetValue());
                           }
                           return true;
                   }

                   if (MaterialInput.IsConnected() && MaterialInput.Expression)
                   {
                           return ConnectMaterialInput(&MaterialInput, ShaderInputName);
                   }

                   if (MaterialInput.UseConstant)
                   {
                           return SetSurfaceShaderInputFromColorConstant(ShaderInputName, MaterialInput.Constant);
                   }

                   if (FallbackValue.IsSet())
                   {
                           return SetSurfaceShaderInputFromColorConstant(ShaderInputName, FallbackValue.GetValue());
                   }

                   return true;
           };

   auto ConnectVectorMaterialInput = [&](EMaterialProperty Property, const FVectorMaterialInput& MaterialInput, const pxr::TfToken& ShaderInputName,
           const TOptional<FVector3f>& FallbackValue = TOptional<FVector3f>()) -> bool
           {
                   if (!IsMaterialPropertyActiveForExport(Material, Property))
                   {
                           if (FallbackValue.IsSet())
                           {
                                   return SetSurfaceShaderInputFromVectorConstant(ShaderInputName, FallbackValue.GetValue());
                           }
                           return true;
                   }

                   if (MaterialInput.IsConnected() && MaterialInput.Expression)
                   {
                           return ConnectMaterialInput(&MaterialInput, ShaderInputName);
                   }

                   if (MaterialInput.UseConstant)
                   {
                           return SetSurfaceShaderInputFromVectorConstant(ShaderInputName, MaterialInput.Constant);
                   }

                   if (FallbackValue.IsSet())
                   {
                           return SetSurfaceShaderInputFromVectorConstant(ShaderInputName, FallbackValue.GetValue());
                   }

                   return true;
           };

	// Connect inputs based on shading model
	if (ShadingModel == EMaterialShadingModel::MSM_DefaultLit || ShadingModel == EMaterialShadingModel::MSM_ClearCoat)
	{
		// UsdPreviewSurface connections
           bAllConnectionsSucceeded &= ConnectColorMaterialInput(MP_BaseColor, EditorOnly->BaseColor, pxr::TfToken("diffuseColor"), FLinearColor(0.0f, 0.0f, 0.0f));
           bAllConnectionsSucceeded &= ConnectScalarMaterialInput(MP_Metallic, EditorOnly->Metallic, pxr::TfToken("metallic"), 0.0f);
           bAllConnectionsSucceeded &= ConnectScalarMaterialInput(MP_Specular, EditorOnly->Specular, pxr::TfToken("specularColor"), 0.5f);
           bAllConnectionsSucceeded &= ConnectScalarMaterialInput(MP_Roughness, EditorOnly->Roughness, pxr::TfToken("roughness"), 0.5f);
           bAllConnectionsSucceeded &= ConnectColorMaterialInput(MP_EmissiveColor, EditorOnly->EmissiveColor, pxr::TfToken("emissiveColor"), FLinearColor(0.0f, 0.0f, 0.0f));
           bAllConnectionsSucceeded &= ConnectVectorMaterialInput(MP_Normal, EditorOnly->Normal, pxr::TfToken("normal"), FVector3f(0.0f, 0.0f, 1.0f));
           bAllConnectionsSucceeded &= (Material->GetBlendMode() == BLEND_Masked
                           ? ConnectScalarMaterialInput(MP_OpacityMask, EditorOnly->OpacityMask, pxr::TfToken("opacity"), 1.0f)
                           : ConnectScalarMaterialInput(MP_Opacity, EditorOnly->Opacity, pxr::TfToken("opacity"), 1.0f));
           bAllConnectionsSucceeded &= ConnectScalarMaterialInput(MP_AmbientOcclusion, EditorOnly->AmbientOcclusion, pxr::TfToken("occlusion"), 1.0f);
           bAllConnectionsSucceeded &= ConnectScalarMaterialInput(MP_CustomData0, EditorOnly->ClearCoat, pxr::TfToken("clearcoat"),
                           ShadingModel == EMaterialShadingModel::MSM_ClearCoat ? TOptional<float>(1.0f) : TOptional<float>());
           bAllConnectionsSucceeded &= ConnectScalarMaterialInput(MP_CustomData1, EditorOnly->ClearCoatRoughness, pxr::TfToken("clearcoatRoughness"),
                           ShadingModel == EMaterialShadingModel::MSM_ClearCoat ? TOptional<float>(0.1f) : TOptional<float>());
           bAllConnectionsSucceeded &= ConnectScalarMaterialInput(MP_Refraction, EditorOnly->Refraction, pxr::TfToken("ior"), 1.0f);
           bAllConnectionsSucceeded &= ConnectScalarMaterialInput(MP_Displacement, EditorOnly->Displacement, pxr::TfToken("displacement"));
	}
	else if (ShadingModel == EMaterialShadingModel::MSM_Unlit)
	{
		// RealityKit Unlit connections
           bAllConnectionsSucceeded &= ConnectColorMaterialInput(MP_EmissiveColor, EditorOnly->EmissiveColor, pxr::TfToken("color"), FLinearColor(0.0f, 0.0f, 0.0f));
           bAllConnectionsSucceeded &= (Material->GetBlendMode() == BLEND_Masked
                           ? ConnectScalarMaterialInput(MP_OpacityMask, EditorOnly->OpacityMask, pxr::TfToken("opacity"), 1.0f)
                           : ConnectScalarMaterialInput(MP_Opacity, EditorOnly->Opacity, pxr::TfToken("opacity"), 1.0f));
	}
	
	if (!bAllConnectionsSucceeded)
	{
		if (LastError.IsEmpty())
		{
			SetFailure(TEXT("ConnectMaterialProperties"), TEXT("One or more material properties failed to connect"));
		}
	}
	
	return bAllConnectionsSucceeded;
}

FString FShaderGraphUsdConvertorPICO::GetTexturePathFromTexture(UTexture* Texture, pxr::UsdStageRefPtr Stage)
{
	if (!Texture || !Stage)
	{
		return FString();
	}

	FString TexturePackagePath = Texture->GetPathName();

	// Remove the object name from the path if it's duplicated
	// GetPathName() returns format like: /Game/Textures/MyTexture.MyTexture
	// We need to extract just the package path without the object name
	FString PackageName;
	FString ObjectName;
	TexturePackagePath.Split(TEXT("."), &PackageName, &ObjectName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	// Non-uasset exports can be redirected to Build/Android/assets/AssetsPICO when APK-assets loading is enabled.
	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	const FString AssetsRootPath = (Settings && Settings->bLoadAssetsFromApkAssets)
		? FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), TEXT("Build/Android/assets/AssetsPICO"))
		: FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()), TEXT("AssetsPICO"));
	const TArray<FString, TInlineAllocator<2>> TextureExtensions = { TEXT(".PNG"), TEXT(".EXR") };
	FString TextureFilePath = FPaths::ConvertRelativePathToFull(
		AssetsRootPath / PackageName + TextureExtensions[0]);
	for (const FString& Extension : TextureExtensions)
	{
		const FString CandidateTextureFilePath = FPaths::ConvertRelativePathToFull(
			AssetsRootPath / PackageName + Extension);
		if (IFileManager::Get().FileExists(*CandidateTextureFilePath))
		{
			TextureFilePath = CandidateTextureFilePath;
			break;
		}
	}

	pxr::SdfLayerHandle RootLayer = Stage->GetRootLayer();
	if (!RootLayer)
	{
		return FString();
	}

	std::string UsdLayerPath = RootLayer->GetRealPath();
	if (UsdLayerPath.empty())
	{
		UsdLayerPath = RootLayer->GetIdentifier();
	}

	const FString UsdFullPath = FPaths::ConvertRelativePathToFull(FString(ANSI_TO_TCHAR(UsdLayerPath.c_str())));
	const FString UsdDir = FPaths::CreateStandardFilename(FPaths::GetPath(UsdFullPath));

	FString TexturePath;
	const FString TextureDir = FPaths::CreateStandardFilename(FPaths::GetPath(TextureFilePath));

	if (TextureDir.Equals(UsdDir, ESearchCase::IgnoreCase))
	{
		TexturePath = FPaths::GetCleanFilename(TextureFilePath);
	}
	else
	{
		TexturePath = FPaths::CreateStandardFilename(TextureFilePath);
		FString BaseDir = UsdDir;
		if (!BaseDir.EndsWith(TEXT("/")))
		{
			BaseDir += TEXT("/");
		}
		if (!FPaths::MakePathRelativeTo(TexturePath, *BaseDir))
		{
			TexturePath = FPaths::GetCleanFilename(TextureFilePath);
		}
	}

	return TexturePath;
}

#endif // USE_USD_SDK
