// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PXR_ProceduralMeshComponent.h"
#include "PXR_MRTypes.h"
#include "PXR_SpatialMeshComponent.generated.h"


UCLASS(ClassGroup = "PICO")
class UPICOSpatialMeshComponent : public UPICOProceduralMeshComponent
{
	GENERATED_BODY()
public:

	UPICOSpatialMeshComponent(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintPure, Category = "PICO|PICO XR Toolkit|Spatial Mesh Actor")
	EPICOSemanticLabel GetSemanticByIndex(int32 Index);

	void AddIndexToSemanticLabel(int32 Index,EPICOSemanticLabel SceneLabel);
	
protected:
	TMap<int32,EPICOSemanticLabel> IndexToSemanticLabelMap; 
};
