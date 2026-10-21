#pragma once

#include "PICOSpatialShaderGraphUsdConvertor.h"

#if USE_USD_SDK
class FExpressionDDXPICO : public FExpressionBasePICO
{
public:
	FExpressionDDXPICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;

	// MaterialX ND_ddx_* only exists for float/vector variants.
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

class FExpressionDDYPICO : public FExpressionBasePICO
{
public:
	FExpressionDDYPICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;

	// MaterialX ND_ddy_* only exists for float/vector variants.
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
