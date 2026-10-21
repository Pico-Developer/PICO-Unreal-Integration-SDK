#pragma once

#include "PICOSpatialShaderGraphUsdConvertor.h"

#if USE_USD_SDK
class FExpressionIfPICO : public FExpressionBasePICO
{
public:
	FExpressionIfPICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;

private:
	pxr::UsdShadeNodeGraph NodeGraph;
	pxr::UsdShadeShader IfEqualShader;
	pxr::UsdShadeShader IfGreaterOrEqualShader;
};
#endif // USE_USD_SDK
