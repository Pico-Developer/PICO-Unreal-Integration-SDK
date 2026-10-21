#pragma once

#include "PICOSpatialShaderGraphUsdConvertor.h"

#if USE_USD_SDK
class FExpressionMFPIPICO : public FExpressionBasePICO
{
public:
	FExpressionMFPIPICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;

private:
	pxr::UsdShadeNodeGraph NodeGraph;
};
#endif // USE_USD_SDK
