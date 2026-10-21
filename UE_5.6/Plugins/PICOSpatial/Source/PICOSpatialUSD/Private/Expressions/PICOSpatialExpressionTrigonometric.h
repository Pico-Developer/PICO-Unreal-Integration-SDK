#pragma once

#include "PICOSpatialShaderGraphUsdConvertor.h"

#if USE_USD_SDK
class FExpressionSinePICO : public FExpressionBasePICO
{
public:
	FExpressionSinePICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;

private:
	pxr::UsdShadeNodeGraph NodeGraph;
};

class FExpressionCosinePICO : public FExpressionBasePICO
{
public:
	FExpressionCosinePICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;

private:
	pxr::UsdShadeNodeGraph NodeGraph;
};

class FExpressionArcsinePICO : public FExpressionBasePICO
{
public:
	FExpressionArcsinePICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;
};

class FExpressionArccosinePICO : public FExpressionBasePICO
{
public:
	FExpressionArccosinePICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;
};

class FExpressionTangentPICO : public FExpressionBasePICO
{
public:
	FExpressionTangentPICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;

private:
	pxr::UsdShadeNodeGraph NodeGraph;
};

class FExpressionArctangent2PICO : public FExpressionBasePICO
{
public:
	FExpressionArctangent2PICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression)
		: FExpressionBasePICO(InConvertor, InExpression)
	{
	}

	virtual void DefineShader(pxr::UsdStageRefPtr Stage) override;
	virtual bool ConnectInput() override;
};
#endif // USE_USD_SDK
