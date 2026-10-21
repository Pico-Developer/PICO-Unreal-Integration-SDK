#pragma once

#include "PICOSpatialShaderGraphUsdConvertor.h"

#if USE_USD_SDK
class FExpressionNormalizePICO : public FExpressionBasePICO
{
public:
	FExpressionNormalizePICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;

	// MaterialX ND_normalize_* only exists for vector2/vector3/vector4 â€?	// no float variant and no color variant.
	// ResolveOperatingType() uses this list to:
	//   â€?Raise float inputs up to vector2 (minimum).
	//   â€?Clamp vector4/color4 inputs down to vector4 (maximum) â€?already covered.
	//   â€?Downcast color inputs to their vector equivalent dimension
	//     (Color3f has dim 3 â†?resolved to Float3; handled in DefineShader via
	//     ToVectorEquivalent, ConnectInput inserts a convert if output is color).
	virtual TArray<pxr::SdfValueTypeName> GetSupportedVariants() const override
	{
		return TArray<pxr::SdfValueTypeName>({
			pxr::SdfValueTypeNames->Float2,
			pxr::SdfValueTypeNames->Float3,
			pxr::SdfValueTypeNames->Float4,
		});
	}
};
#endif // USE_USD_SDK
