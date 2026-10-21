// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PICO_MRTypes.h"
#include "PICO_ProceduralMeshComponent.h"
#include "PICO_SpatialMeshComponent.generated.h"

UCLASS(ClassGroup = "PICO")
class USpatialMeshComponentPICO : public UProceduralMeshComponentPICO
{
	GENERATED_BODY()
public:

	USpatialMeshComponentPICO(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintPure, Category = "PICO|PICO XR Toolkit|Spatial Mesh Actor")
	ESemanticLabelPICO GetSemanticByIndex(int32 Index);

	void AddIndexToSemanticLabel(int32 Index,ESemanticLabelPICO SceneLabel);
	
protected:
	TMap<int32,ESemanticLabelPICO> IndexToSemanticLabelMap; 
};