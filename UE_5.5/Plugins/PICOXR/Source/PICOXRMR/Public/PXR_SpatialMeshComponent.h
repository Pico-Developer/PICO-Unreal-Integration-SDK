// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "PXR_MRTypes.h"
#include "PXR_SpatialMeshComponent.generated.h"

UCLASS(hidecategories = (Object, LOD), meta = (BlueprintSpawnableComponent), ClassGroup = Rendering)
class UPICOSpatialMeshComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()
public:

	UPICOSpatialMeshComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "PICO XR Toolkit")
	EPICOSemanticLabel GetSemanticByIndex(int32 Index);

	void AddIndexToSemanticLabel(int32 Index,EPICOSemanticLabel SceneLabel);

	void SetUpdateTime(uint64 Time);

	int64 GetUpdateTime() const;

	TArray<int32> GetCachedIndices() { return CachedIndices; };

	bool IsEqualWithCached(TArray<int32>& Indices);

	void SetCachedIndices(const TArray<int32>& Indices) {CachedIndices = Indices; };
	
protected:
	TMap<int32,EPICOSemanticLabel> IndexToAnchorSceneLabelMap; //Index ->SceneLabel
	uint64 LastUpdateTime = 0;

	TArray<int32> CachedIndices;
};