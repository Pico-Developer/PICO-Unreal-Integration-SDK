#pragma once

#include "PICOSpatialShaderGraphUsdConvertor.h"

#if USE_USD_SDK
class FExpressionSquareRootPICO : public FExpressionBasePICO
{
public:
	FExpressionSquareRootPICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;

	// MaterialX ND_sqrt_* only exists for float/vector variants.
	// Color inputs therefore resolve to the same-dimension vector variant and
	// are converted during ConnectInput().
	virtual TArray<pxr::SdfValueTypeName> GetSupportedVariants() const override
	{
		return TArray<pxr::SdfValueTypeName>({
			pxr::SdfValueTypeNames->Float,
			pxr::SdfValueTypeNames->Float2,
			pxr::SdfValueTypeNames->Float3,
			pxr::SdfValueTypeNames->Float4,
		});
	}
};
#endif // USE_USD_SDK
