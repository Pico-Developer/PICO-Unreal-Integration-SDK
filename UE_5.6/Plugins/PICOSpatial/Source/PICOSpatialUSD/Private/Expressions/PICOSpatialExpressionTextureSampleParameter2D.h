#pragma once

#include "PICOSpatialExpressionTextureObject.h"

#if USE_USD_SDK
class FExpressionTextureSampleParameter2DPICO : public FExpressionTextureBasePICO
{
public:
    FExpressionTextureSampleParameter2DPICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
        : FExpressionTextureBasePICO(InConvertor, InExpression)
    {
    }

    virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
    virtual bool ConnectInput() override;
};
#endif // USE_USD_SDK
