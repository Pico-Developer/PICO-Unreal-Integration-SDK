#pragma once

#include "PICOSpatialShaderGraphUsdConvertor.h"

#if USE_USD_SDK
class FExpressionTextureBasePICO : public FExpressionBasePICO
{
public:
    FExpressionTextureBasePICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
        :FExpressionBasePICO(InConvertor, InExpression)
    {
    }

protected:
    /**
     * Check if the texture is supported for USD export
     * @param Texture The texture to check
     * @return true if supported, false if not (e.g., RenderTarget textures)
     */
    static bool IsTextureSupported(UTexture* Texture);
};

class FExpressionTextureObjectPICO: public FExpressionTextureBasePICO
{
public:
    FExpressionTextureObjectPICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
        :FExpressionTextureBasePICO(InConvertor, InExpression)
    {
    }

    virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
};
#endif // USE_USD_SDK
