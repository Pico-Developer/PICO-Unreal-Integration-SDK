#pragma once

#include "PICOSpatialShaderGraphUsdConvertor.h"

#if USE_USD_SDK
class FExpressionVectorParameterPICO : public FExpressionBasePICO
{
public:
    FExpressionVectorParameterPICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
        :FExpressionBasePICO(InConvertor, InExpression)
    {
    }

    virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
};
#endif // USE_USD_SDK
