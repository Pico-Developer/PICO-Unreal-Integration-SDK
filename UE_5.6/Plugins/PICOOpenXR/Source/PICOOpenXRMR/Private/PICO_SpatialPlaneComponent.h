// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PICO_ProceduralMeshComponent.h"
#include "PICO_SpatialPlaneComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = "PICO")
class PICOOPENXRMR_API USpatialPlaneComponentPICO : public UProceduralMeshComponentPICO
{
	GENERATED_BODY()

public:
	USpatialPlaneComponentPICO(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintPure, Category = "PICO|PICO XR Toolkit|Spatial Plane Actor")
	ESemanticLabelPICO GetPlaneSemantic();
	
	void SetPlaneSemantic(const ESemanticLabelPICO& SemanticLabel);

	UFUNCTION(BlueprintPure, Category = "PICO|PICO XR Toolkit|Spatial Plane Actor")
	EPlaneOrientationPICO GetPlaneOrientation() const;

	void SetPlaneOrientation(const EPlaneOrientationPICO& SemanticLabel);

private:
	ESemanticLabelPICO SpatialPlaneSemantic;
	EPlaneOrientationPICO SpatialPlaneOrientation;
};
