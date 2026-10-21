#pragma once

#include "PICOSpatialExpressionTextureObject.h"

#if USE_USD_SDK
class FExpressionTextureSamplePICO : public FExpressionTextureBasePICO
{
public:
    FExpressionTextureSamplePICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
        : FExpressionTextureBasePICO(InConvertor, InExpression)
    {
    }

    virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
    virtual bool ConnectInput() override;
};
#endif // USE_USD_SDK
