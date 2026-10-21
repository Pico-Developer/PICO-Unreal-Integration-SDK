// Fill out your copyright notice in the Description page of Project Settings.

#include "PICO_PassthroughLayer.h"
#include "PICO_PassthroughLayerShape.h"
#include "Curves/CurveLinearColor.h"
#include "PICO_PassthroughModule.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"

void UPassthroughFullScreenLayerPICO::ApplyShape(IStereoLayers::FLayerDesc& LayerDesc)
{
	const FVSTEdgeStyleParametersPICO EdgeStyleParameters(bEnableEdgeColor, bEnableColorMap, TextureOpacityFactor, Brightness, Contrast, Posterize, Saturation, EdgeColor, ColorScale, ColorOffset, ColorMapType, GetColorMapGradient(bUseColorMapCurve, ColorMapCurve));
	LayerDesc.SetShape<FFullScreenLayerShapePICO>(EdgeStyleParameters, LayerOrder);
}

void UPassthroughCustomShapeLayerPICO::AddGeometry(const FString& MeshName, FPICOPassthroughMeshRef PassthroughMesh, FTransform Transform, bool bUpdateTransform)
{
	FCustomShapeGeometryDescPICO CustomShapeGeometryDesc(
		MeshName,
		PassthroughMesh,
		Transform,
		bUpdateTransform);

	UserGeometryList.Add(CustomShapeGeometryDesc);
}

void UPassthroughCustomShapeLayerPICO::RemoveGeometry(const FString& MeshName)
{
	UserGeometryList.RemoveAll([MeshName](const FCustomShapeGeometryDescPICO& Desc) {
		return Desc.MeshName == MeshName;
		});
}

void UPassthroughCustomShapeLayerPICO::ApplyShape(IStereoLayers::FLayerDesc& LayerDesc)
{
	const FVSTEdgeStyleParametersPICO EdgeStyleParameters(bEnableEdgeColor, bEnableColorMap, TextureOpacityFactor, Brightness, Contrast, Posterize, Saturation, EdgeColor, ColorScale, ColorOffset, ColorMapType, GetColorMapGradient(bUseColorMapCurve, ColorMapCurve));
	LayerDesc.SetShape<FCustomShapeLayerShapePICO>(UserGeometryList, EdgeStyleParameters, LayerOrder);
}

void UPassthroughLayerBasePICO::SetTextureOpacity(float InOpacity)
{
	if (TextureOpacityFactor == InOpacity)
	{
		return;
	}

	TextureOpacityFactor = InOpacity;
	MarkStereoLayerDirty();
}

void UPassthroughLayerBasePICO::EnableEdgeColor(bool bInEnableEdgeColor)
{
	if (bEnableEdgeColor == bInEnableEdgeColor)
	{
		return;
	}
	bEnableEdgeColor = bInEnableEdgeColor;
	MarkStereoLayerDirty();
}

void UPassthroughLayerBasePICO::EnableColorMap(bool bInEnableColorMap)
{
	if (bEnableColorMap == bInEnableColorMap)
	{
		return;
	}
	bEnableColorMap = bInEnableColorMap;
	MarkStereoLayerDirty();
}

void UPassthroughLayerBasePICO::EnableColorMapCurve(bool bInEnableColorMapCurve)
{
	if (bUseColorMapCurve == bInEnableColorMapCurve)
	{
		return;
	}
	bUseColorMapCurve = bInEnableColorMapCurve;
	ColorMapGradient = GenerateColorMapGradient(bUseColorMapCurve, ColorMapCurve);
	MarkStereoLayerDirty();
}

void UPassthroughLayerBasePICO::SetEdgeRenderingColor(FLinearColor InEdgeColor)
{
	if (EdgeColor == InEdgeColor)
	{
		return;
	}
	EdgeColor = InEdgeColor;
	MarkStereoLayerDirty();
}

void UPassthroughLayerBasePICO::SetColorMapControls(float InContrast, float InBrightness, float InPosterize)
{
	if (ColorMapType != EColorMapTypePICO::ColorMapType_Grayscale && ColorMapType != EColorMapTypePICO::ColorMapType_GrayscaleToColor) {
		UE_LOG(LogPassthroughPICO, Warning, TEXT("SetColorMapControls is ignored for color map types other than Grayscale and Grayscale to color."));
		return;
	}
	Contrast = FMath::Clamp(InContrast, -1.0f, 1.0f);
	Brightness = FMath::Clamp(InBrightness, -1.0f, 1.0f);
	Posterize = FMath::Clamp(InPosterize, 0.0f, 1.0f);

	MarkStereoLayerDirty();
}

void UPassthroughLayerBasePICO::SetBrightnessContrastSaturation(float InContrast, float InBrightness, float InSaturation)
{
	if (ColorMapType != EColorMapTypePICO::ColorMapType_ColorAdjustment) {
		UE_LOG(LogPassthroughPICO, Warning, TEXT("SetBrightnessContrastSaturation is ignored for color map types other than Color Adjustment."));
		return;
	}
	Contrast = FMath::Clamp(InContrast, -1.0f, 1.0f);
	Brightness = FMath::Clamp(InBrightness, -1.0f, 1.0f);
	Saturation = FMath::Clamp(InSaturation, -1.0f, 1.0f);

	MarkStereoLayerDirty();
}

void UPassthroughLayerBasePICO::SetColorScaleAndOffset(FLinearColor InColorScale, FLinearColor InColorOffset)
{
	if (ColorScale == InColorScale && ColorOffset == InColorOffset)
	{
		return;
	}
	ColorScale = InColorScale;
	ColorOffset = InColorOffset;
	MarkStereoLayerDirty();
}

void UPassthroughLayerBasePICO::SetColorMapCurve(UCurveLinearColor* InColorMapCurve)
{
	if (ColorMapCurve == InColorMapCurve)
	{
		return;
	}
	ColorMapCurve = InColorMapCurve;
	ColorMapGradient = GenerateColorMapGradient(bUseColorMapCurve, ColorMapCurve);
	MarkStereoLayerDirty();
}

void UPassthroughLayerBasePICO::SetColorMapGradient(const TArray<FLinearColor>& InColorMapGradient)
{
	if (InColorMapGradient.Num() == 0)
	{
		return;
	}

	if (ColorMapType != EColorMapTypePICO::ColorMapType_GrayscaleToColor) {
		UE_LOG(LogPassthroughPICO, Warning, TEXT("SetColorMapGradient is ignored for color map types other than Grayscale to Color."));
		return;
	}

	if (bUseColorMapCurve)
	{
		UE_LOG(LogPassthroughPICO, Warning, TEXT("UseColorMapCurve is enabled on the layer. Automatic disable and use the Array for color lookup"));
	}
	bUseColorMapCurve = false;

	ColorMapGradient = InColorMapGradient;

	MarkStereoLayerDirty();
}

void UPassthroughLayerBasePICO::ClearColorMap()
{
	ColorMapGradient.Empty();
}

void UPassthroughLayerBasePICO::SetColorMapType(EColorMapTypePICO InColorMapType)
{
	if (ColorMapType == InColorMapType)
	{
		return;
	}
	ColorMapType = InColorMapType;
	MarkStereoLayerDirty();
}

void UPassthroughLayerBasePICO::SetLayerPlacement(EPassthroughLayerOrderPICO InLayerOrder)
{
	if (LayerOrder == InLayerOrder)
	{
		UE_LOG(LogPassthroughPICO, Warning, TEXT("Same layer order as before, no change needed"));
		return;
	}

	LayerOrder = InLayerOrder;
	this->MarkStereoLayerDirty();
}

void UPassthroughLayerBasePICO::MarkStereoLayerDirty()
{
	check(GetOuter()->IsA<UPassthroughLayerComponentPICO>());
	Cast<UPassthroughLayerComponentPICO>(GetOuter())->MarkStereoLayerDirty();
}

TArray<FLinearColor> UPassthroughLayerBasePICO::GenerateColorGradientFromColorCurve(const UCurveLinearColor* InColorMapCurve) const
{
	if (InColorMapCurve == nullptr)
	{
		return TArray<FLinearColor>();
	}

	TArray<FLinearColor> Gradient;
	constexpr uint32 TotalEntries = 256;
	Gradient.Empty();
	Gradient.SetNum(TotalEntries);

	for (int32 Index = 0; Index < TotalEntries; ++Index)
	{
		const float Alpha = ((float)Index / TotalEntries);
		Gradient[Index] = InColorMapCurve->GetLinearColorValue(Alpha);
	}
	return Gradient;
}

TArray<FLinearColor> UPassthroughLayerBasePICO::GetOrGenerateNeutralColorMapGradient()
{
	if (NeutralMapGradient.Num() == 0) {
		const uint32 TotalEntries = 256;
		NeutralMapGradient.SetNum(TotalEntries);

		for (int32 Index = 0; Index < TotalEntries; ++Index)
		{
			NeutralMapGradient[Index] = FLinearColor((float)Index / TotalEntries, (float)Index / TotalEntries, (float)Index / TotalEntries);
		}
	}

	return NeutralMapGradient;
}

TArray<FLinearColor> UPassthroughLayerBasePICO::GetColorMapGradient(bool bInUseColorMapCurve, const UCurveLinearColor* InColorMapCurve)
{
	if (ColorMapGradient.Num() == 0) {
		if (bInUseColorMapCurve) {
			return GenerateColorMapGradient(bInUseColorMapCurve, InColorMapCurve);
		}
		return GetOrGenerateNeutralColorMapGradient();
	}

	return ColorMapGradient;
}

TArray<FLinearColor> UPassthroughLayerBasePICO::GenerateColorMapGradient(bool bInUseColorMapCurve, const UCurveLinearColor* InColorMapCurve)
{
	TArray<FLinearColor> NewColorGradient;
	if (bInUseColorMapCurve)
	{
		NewColorGradient = GenerateColorGradientFromColorCurve(InColorMapCurve);
	}
	// Check for existing gradient, otherwise generate a neutral one
	if (NewColorGradient.Num() == 0)
	{
		NewColorGradient = GetOrGenerateNeutralColorMapGradient();
	}
	return NewColorGradient;
}

UPassthroughLayerComponentPICO::UPassthroughLayerComponentPICO(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, passthroughLayerId(0)
	, bIsDirty(true)
{
}
void UPassthroughLayerComponentPICO::OnUnregister()
{
	Super::OnUnregister();

	passthroughLayerId = FPICOOpenXRPassthroughModule::Get().GetPassthroughPlugin().DestroyPassthroughLayer(passthroughLayerId);
	passthroughLayerId = 0;
}

void UPassthroughLayerComponentPICO::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	UpdatePassthroughObjects();

	FTransform Transform;
	if (StereoLayerType == SLT_WorldLocked)
	{
		Transform = GetComponentTransform();
	}
	else
	{
		Transform = GetRelativeTransform();
	}

	if (!bIsDirty && (bLastVisible != GetVisibleFlag() || FMemory::Memcmp(&LastTransform, &Transform, sizeof(Transform)) != 0))
	{
		bIsDirty = true;
	}

	bool bCurrVisible = GetVisibleFlag();

	if (bIsDirty)
	{
		IStereoLayers::FLayerDesc LayerDesc;
		LayerDesc.Priority = Priority;
		LayerDesc.QuadSize = QuadSize;
		LayerDesc.UVRect = UVRect;
		LayerDesc.Transform = Transform;

		LayerDesc.Flags |= (bLiveTexture) ? IStereoLayers::LAYER_FLAG_TEX_CONTINUOUS_UPDATE : 0;
		LayerDesc.Flags |= (bNoAlphaChannel) ? IStereoLayers::LAYER_FLAG_TEX_NO_ALPHA_CHANNEL : 0;
		LayerDesc.Flags |= (bQuadPreserveTextureRatio) ? IStereoLayers::LAYER_FLAG_QUAD_PRESERVE_TEX_RATIO : 0;
		LayerDesc.Flags |= (bSupportsDepth) ? IStereoLayers::LAYER_FLAG_SUPPORT_DEPTH : 0;
		LayerDesc.Flags |= (!bCurrVisible) ? IStereoLayers::LAYER_FLAG_HIDDEN : 0;

		switch (StereoLayerType)
		{
		case SLT_WorldLocked:
			LayerDesc.PositionType = IStereoLayers::WorldLocked;
			break;
		case SLT_TrackerLocked:
			LayerDesc.PositionType = IStereoLayers::TrackerLocked;
			break;
		case SLT_FaceLocked:
			LayerDesc.PositionType = IStereoLayers::FaceLocked;
			break;
		}

		if (Shape == nullptr || (!Shape->IsA<UPassthroughFullScreenLayerPICO>() && !Shape->IsA<UPassthroughCustomShapeLayerPICO>()))
		{
			return;
		}

		Shape->ApplyShape(LayerDesc);

		if (passthroughLayerId)
		{
			FPICOOpenXRPassthroughModule::Get().GetPassthroughPlugin().SetLayerDesc(passthroughLayerId, LayerDesc);
		}
		else
		{
			passthroughLayerId = FPICOOpenXRPassthroughModule::Get().GetPassthroughPlugin().CreatePassthroughLayer(LayerDesc);
		}

		LastTransform = Transform;
		bLastVisible = bCurrVisible;
		bIsDirty = false;

		static uint8 TryAgainCount = 3;
		if (passthroughLayerId == 0 && TryAgainCount > 0)
		{
			UE_LOG(LogPassthroughPICO, Warning, TEXT("Failed to create Passthrough Layer, retrying..."));
			MarkStereoLayerDirty();
			TryAgainCount--;
		}
		else if (passthroughLayerId == 0)
		{
			UE_LOG(LogPassthroughPICO, Error, TEXT("Failed to create Passthrough Layer after retries."));
		}
		else
		{
			TryAgainCount = 3; // Reset the retry count
		}
	}
}

void UPassthroughLayerComponentPICO::UpdatePassthroughObjects()
{
	UPassthroughCustomShapeLayerPICO* UserShape = Cast<UPassthroughCustomShapeLayerPICO>(Shape);
	if (UserShape)
	{
		bool bDirty = false;
		for (FCustomShapeGeometryDescPICO& Entry : UserShape->GetUserGeometryList())
		{
			if (Entry.bUpdateTransform)
			{
				AStaticMeshActor** StaticMeshActor = PassthroughActorMap.Find(Entry.MeshName);
				if (StaticMeshActor)
				{
					UStaticMeshComponent* StaticMeshComponent = (*StaticMeshActor)->GetStaticMeshComponent();
					if (StaticMeshComponent)
					{
						Entry.Transform = StaticMeshComponent->GetComponentTransform();
						bDirty = true;
					}
				}
			}
		}
		if (bDirty) {
			MarkStereoLayerDirty();
		}
	}
}

void UPassthroughLayerComponentPICO::AddSurfaceGeometryPICO(AStaticMeshActor* StaticMeshActor, bool updateTransform)
{
	if (StaticMeshActor == nullptr)
	{
		UE_LOG(LogPassthroughPICO, Error, TEXT("StaticMeshActor is NULL."));
		return;
	}

	UPassthroughCustomShapeLayerPICO* UserShape = Cast<UPassthroughCustomShapeLayerPICO>(Shape);
	if (UserShape == nullptr)
	{
		UE_LOG(LogPassthroughPICO, Warning, TEXT("Surface geometry can be only assign to passthrough layer with `User Defined` shape."));
		return;
	}

	UStaticMeshComponent* StaticMeshComponent = StaticMeshActor->GetStaticMeshComponent();
	if (StaticMeshComponent == nullptr || StaticMeshComponent->GetStaticMesh() == nullptr)
	{
		UE_LOG(LogPassthroughPICO, Warning, TEXT("Static mesh actor does not have mesh component."));
		PassthroughActorMap.Add(StaticMeshActor->GetFullName(), StaticMeshActor);
		MarkStereoLayerDirty();
		return;
	}

	FPICOPassthroughMeshRef PassthroughMesh = CreatePassthroughMesh(StaticMeshComponent->GetStaticMesh());
	if (PassthroughMesh)
	{
		const FString MeshName = StaticMeshActor->GetFullName();
		const FTransform Transform = StaticMeshComponent->GetComponentTransform();
		UserShape->AddGeometry(MeshName, PassthroughMesh, Transform, updateTransform);
	}

	PassthroughActorMap.Add(StaticMeshActor->GetFullName(), StaticMeshActor);
	MarkStereoLayerDirty();
}

void UPassthroughLayerComponentPICO::RemoveSurfaceGeometryPICO(AStaticMeshActor* StaticMeshActor)
{
	if (StaticMeshActor)
	{
		UPassthroughCustomShapeLayerPICO* UserShape = Cast<UPassthroughCustomShapeLayerPICO>(Shape);
		if (UserShape)
		{
			UStaticMeshComponent* StaticMeshComponent = StaticMeshActor->GetStaticMeshComponent();
			if (StaticMeshComponent)
			{
				UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
				if (StaticMesh)
				{
					const FString MeshName = StaticMeshActor->GetFullName();
					UserShape->RemoveGeometry(MeshName);
				}
			}
			PassthroughActorMap.Remove(StaticMeshActor->GetFullName());
		}
	}

	MarkStereoLayerDirty();
}

bool UPassthroughLayerComponentPICO::IsSurfaceGeometryPICO(AStaticMeshActor* StaticMeshActor) const
{
	if (StaticMeshActor)
	{
		UPassthroughCustomShapeLayerPICO* UserShape = Cast<UPassthroughCustomShapeLayerPICO>(Shape);
		if (UserShape)
		{
			return PassthroughActorMap.Contains(StaticMeshActor->GetFullName());
		}
	}

	return false;
}

void UPassthroughLayerComponentPICO::MarkPassthroughStyleForUpdatePICO()
{
	bPassthroughStyleNeedsUpdate = true;
}

bool UPassthroughLayerComponentPICO::PausePassthroughPICO()
{
	return 	FPICOOpenXRPassthroughModule::Get().GetPassthroughPlugin().PausePassthroughLayer(passthroughLayerId);;
}

bool UPassthroughLayerComponentPICO::ResumePassthroughPICO()
{
	return 	FPICOOpenXRPassthroughModule::Get().GetPassthroughPlugin().ResumePassthroughLayer(passthroughLayerId);;
}

FPICOPassthroughMeshRef UPassthroughLayerComponentPICO::CreatePassthroughMesh(UStaticMesh* Mesh)
{
	if (!Mesh || !Mesh->GetRenderData())
	{
		UE_LOG(LogPassthroughPICO, Error, TEXT("Passthrough Static Mesh has no Renderdata"));
		return nullptr;
	}

	if (Mesh->GetNumLODs() == 0)
	{
		UE_LOG(LogPassthroughPICO, Error, TEXT("Passthrough Static Mesh has no LODs"));
		return nullptr;
	}

	if (!Mesh->bAllowCPUAccess)
	{
		UE_LOG(LogPassthroughPICO, Error, TEXT("Passthrough Static Mesh Requires CPU Access"));
		return nullptr;
	}

	const int32 LODIndex = 0;
	FStaticMeshLODResources& LOD = Mesh->GetRenderData()->LODResources[LODIndex];

	TArray<int32> Triangles;
	const int32 NumIndices = LOD.IndexBuffer.GetNumIndices();
	for (int32 i = 0; i < NumIndices; ++i)
	{
		Triangles.Add(LOD.IndexBuffer.GetIndex(i));
	}

	TArray<FVector> Vertices;
	const int32 NumVertices = LOD.VertexBuffers.PositionVertexBuffer.GetNumVertices();
	for (int32 i = 0; i < NumVertices; ++i)
	{
		Vertices.Add((FVector)LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(i));
	}

	FPICOPassthroughMeshRef PassthroughMesh = new FPassthroughMeshPICO(Vertices, Triangles);
	return PassthroughMesh;
}
