#pragma once

#include "PICOSpatialShaderGraphUsdConvertor.h"

#if USE_USD_SDK
class FExpressionDotProductPICO : public FExpressionBasePICO
{
public:
	FExpressionDotProductPICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;

	// MaterialX ND_dotproduct_* only exists for vector variants. A scalar input is
	// handled via a small NodeGraph emulation that computes multiply(in1, in2).
	virtual TArray<pxr::SdfValueTypeName> GetSupportedVariants() const override
	{
		return TArray<pxr::SdfValueTypeName>({
			pxr::SdfValueTypeNames->Float,
			pxr::SdfValueTypeNames->Float2,
			pxr::SdfValueTypeNames->Float3,
			pxr::SdfValueTypeNames->Float4,
		});
	}

private:
	pxr::UsdShadeNodeGraph NodeGraph;
};
#endif // USE_USD_SDK
