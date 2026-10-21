// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PXR_MRTypes.h"
#include "PXR_ProceduralMeshComponent.h"
#include "PXR_SpatialPlaneComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = "PICO")
class UPICOSpatialPlaneComponent : public UPICOProceduralMeshComponent
{
	GENERATED_BODY()

public:
	UPICOSpatialPlaneComponent(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintPure, Category = "PICO|PICO XR Toolkit|Spatial Plane Actor")
	EPICOSemanticLabel GetPlaneSemantic();
	
	void SetPlaneSemantic(const EPICOSemanticLabel& SemanticLabel);

	UFUNCTION(BlueprintPure, Category = "PICO|PICO XR Toolkit|Spatial Plane Actor")
	EPICOPlaneOrientation GetPlaneOrientation() const;

	void SetPlaneOrientation(const EPICOPlaneOrientation& SemanticLabel);

private:
	EPICOSemanticLabel SpatialPlaneSemantic;
	EPICOPlaneOrientation SpatialPlaneOrientation;
};
