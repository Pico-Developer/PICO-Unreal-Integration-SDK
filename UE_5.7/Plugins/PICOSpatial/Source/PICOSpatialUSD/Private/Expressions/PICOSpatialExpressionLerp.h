#pragma once

#include "PICOSpatialShaderGraphUsdConvertor.h"

#if USE_USD_SDK
class FExpressionLerpPICO : public FExpressionBasePICO
{
public:
	FExpressionLerpPICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;
};
#endif // USE_USD_SDK

