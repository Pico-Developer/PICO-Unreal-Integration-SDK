#pragma once

#include "PICOSpatialShaderGraphUsdConvertor.h"

#if USE_USD_SDK
class FExpressionFloorPICO : public FExpressionBasePICO
{
public:
	FExpressionFloorPICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;
};

class FExpressionCeilPICO : public FExpressionBasePICO
{
public:
	FExpressionCeilPICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;
};
#endif // USE_USD_SDK
