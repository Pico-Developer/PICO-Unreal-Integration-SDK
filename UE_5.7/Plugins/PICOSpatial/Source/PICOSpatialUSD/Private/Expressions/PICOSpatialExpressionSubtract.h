#pragma once

#include "PICOSpatialShaderGraphUsdConvertor.h"

#if USE_USD_SDK
class FExpressionSubtractPICO : public FExpressionBasePICO
{
public:
    FExpressionSubtractPICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
        :FExpressionBasePICO(InConvertor, InExpression)
    {
    }

    virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
    virtual bool ConnectInput() override;
};
#endif // USE_USD_SDK
