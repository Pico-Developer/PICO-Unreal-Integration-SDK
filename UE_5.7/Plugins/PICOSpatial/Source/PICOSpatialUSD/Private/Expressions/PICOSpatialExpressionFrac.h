#pragma once

#include "PICOSpatialShaderGraphUsdConvertor.h"

#if USE_USD_SDK
class FExpressionFracPICO : public FExpressionBasePICO
{
public:
	FExpressionFracPICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;

	// MaterialX ND_fractional_* is available for the standard float/color/vector variants.
	virtual TArray<pxr::SdfValueTypeName> GetSupportedVariants() const override
	{
		return TArray<pxr::SdfValueTypeName>({
			pxr::SdfValueTypeNames->Float,
			pxr::SdfValueTypeNames->Float2,
			pxr::SdfValueTypeNames->Color3f,
			pxr::SdfValueTypeNames->Float3,
			pxr::SdfValueTypeNames->Color4f,
			pxr::SdfValueTypeNames->Float4,
		});
	}
};
#endif // USE_USD_SDK
