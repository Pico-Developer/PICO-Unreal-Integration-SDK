// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StereoLayerShapes.h"
#include "PICO_PassthroughFunctionLibrary.h"
#include "PICO_PassthroughLayerShape.generated.h"

typedef enum {
	NoneStyle = 0,
	MonoToRgba = 1,
	MonoToMono = 2,
	HandsContrast = 3,
	BrightnessContrastSaturation = 4,



	EnumSize = 0x7fffffff
} PassthroughColorMapType;

class FPassthroughMeshPICO : public FRefCountedObject
{
public:
	FPassthroughMeshPICO(const TArray<FVector>& InVertices, const TArray<int32>& InTriangles)
		: Vertices(InVertices)
		, Triangles(InTriangles)
	{
	}

	const TArray<FVector>& GetVertices() const { return Vertices; };
	const TArray<int32>& GetTriangles() const { return Triangles; };

private:
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
};

typedef TRefCountPtr<FPassthroughMeshPICO> FPICOPassthroughMeshRef;


UENUM()
enum class EColorMapTypePICO : uint8
{
	/** None*/
	ColorMapType_None UMETA(DisplayName = "None"),

	/** Grayscale to color */
	ColorMapType_GrayscaleToColor UMETA(DisplayName = "Grayscale To Color"),

	/** Grayscale */
	ColorMapType_Grayscale UMETA(DisplayName = "Grayscale"),

	/** Color Adjustment */
	ColorMapType_ColorAdjustment UMETA(DisplayName = "Color Adjustment"),

	ColorMapType_MAX,
};

UENUM()
enum EPassthroughLayerOrderPICO
{
	/** Layer is rendered under scene */
	PassthroughLayerOrder_Underlay UMETA(DisplayName = "Underlay"),

	/** Layer is rendered on top of scene */
	PassthroughLayerOrder_Overlay UMETA(DisplayName = "Overlay"),

	PassthroughLayerOrder_MAX,
};

struct FVSTEdgeStyleParametersPICO {
	FVSTEdgeStyleParametersPICO()
		: bEnableEdgeColor(false)
		, bEnableColorMap(false)
		, TextureOpacityFactor(1.0f)
		, EdgeColor{}
		, ColorMapType{}
		, ColorMapData{}
	{

	};

	FVSTEdgeStyleParametersPICO(bool bEnableEdgeColor, bool bEnableColorMap, float TextureOpacityFactor, float Brightness, float Contrast, float Posterize, float Saturation
		, FLinearColor EdgeColor, FLinearColor ColorScale, FLinearColor ColorOffset, EColorMapTypePICO InColorMapType, const TArray<FLinearColor>& InColorMapGradient)
		: bEnableEdgeColor(bEnableEdgeColor)
		, bEnableColorMap(bEnableColorMap)
		, TextureOpacityFactor(TextureOpacityFactor)
		, Brightness(Brightness)
		, Contrast(Contrast)
		, Posterize(Posterize)
		, Saturation(Saturation)
		, EdgeColor(EdgeColor)
		, ColorScale(ColorScale)
		, ColorOffset(ColorOffset)
		, ColorMapType(GetPICOColorMapType(InColorMapType))
	{
		ColorMapData = GenerateColorMapData(InColorMapType, InColorMapGradient);
	};

	bool bEnableEdgeColor;
	bool bEnableColorMap;
	float TextureOpacityFactor;
	float Brightness;
	float Contrast;
	float Posterize;
	float Saturation;
	FLinearColor EdgeColor;
	FLinearColor ColorScale;
	FLinearColor ColorOffset;
	PassthroughColorMapType ColorMapType;
	TArray<uint8> ColorMapData;

private:

	/** Generates the corresponding color map based on given color map type */
	TArray<uint8> GenerateColorMapData(EColorMapTypePICO InColorMapType, const TArray<FLinearColor>& InColorMapGradient);

	/** Generates a grayscale to color color map based on given gradient --> It also applies the color scale and offset */
	TArray<uint8> GenerateMonoToRGBA(const TArray<FLinearColor>& InGradient, const TArray<uint8>& InColorMapData);

	/** Generates a grayscale color map with given Brightness/Contrast/Posterize settings */
	TArray<uint8> GenerateMonoBrightnessContrastPosterizeMap();

	/** Generates a luminance based colormap from the the Brightness/Contrast */
	TArray<uint8> GenerateBrightnessContrastSaturationColorMap();

	/** Converts `EColorMapType` to `PassthroughColorMapType` */
	PassthroughColorMapType GetPICOColorMapType(EColorMapTypePICO InColorMapType);
};

class FFullScreenLayerShapePICO : public IStereoLayerShape
{
public:
	PICOOPENXRPASSTHROUGH_API static const FName ShapeName;
	virtual FName GetShapeName() override { return ShapeName; } 
	virtual IStereoLayerShape* Clone() const override { return new FFullScreenLayerShapePICO(*this); }

public:
	FFullScreenLayerShapePICO() {};
	FFullScreenLayerShapePICO(const FVSTEdgeStyleParametersPICO& EdgeStyleParameters, EPassthroughLayerOrderPICO PassthroughLayerOrder)
		: EdgeStyleParameters(EdgeStyleParameters),
		PassthroughLayerOrder(PassthroughLayerOrder)
	{
	};
	FVSTEdgeStyleParametersPICO EdgeStyleParameters;
	EPassthroughLayerOrderPICO PassthroughLayerOrder;
};

struct FCustomShapeGeometryDescPICO
{
	FCustomShapeGeometryDescPICO(const FString& MeshName, FPICOPassthroughMeshRef PassthroughMesh, const FTransform& Transform, bool bUpdateTransform)
		: MeshName(MeshName)
		, PassthroughMesh(PassthroughMesh)
		, Transform(Transform)
		, bUpdateTransform(bUpdateTransform)
	{
	};

	FString MeshName;
	FPICOPassthroughMeshRef PassthroughMesh;
	FTransform  Transform;
	bool bUpdateTransform;
};

class FCustomShapeLayerShapePICO : public IStereoLayerShape
{
public:
	PICOOPENXRPASSTHROUGH_API static const FName ShapeName;
	virtual FName GetShapeName() override { return ShapeName; } 
	virtual IStereoLayerShape* Clone() const override { return new FCustomShapeLayerShapePICO(*this); }

public:
	FCustomShapeLayerShapePICO() {};
	FCustomShapeLayerShapePICO(TArray<FCustomShapeGeometryDescPICO> InUserGeometryList, const FVSTEdgeStyleParametersPICO& EdgeStyleParameters, EPassthroughLayerOrderPICO PassthroughLayerOrder)
		: UserGeometryList{}
		, EdgeStyleParameters(EdgeStyleParameters)
		, PassthroughLayerOrder(PassthroughLayerOrder)
	{
		UserGeometryList = InUserGeometryList;
	}

	TArray<FCustomShapeGeometryDescPICO> UserGeometryList;
	FVSTEdgeStyleParametersPICO EdgeStyleParameters;
	EPassthroughLayerOrderPICO PassthroughLayerOrder;

private:

};