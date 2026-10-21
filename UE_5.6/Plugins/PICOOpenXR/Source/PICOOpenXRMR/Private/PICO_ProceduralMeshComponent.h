// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "PICO_MRTypes.h"
#include "PICO_ProceduralMeshComponent.generated.h"

UCLASS(ClassGroup = "PICO")
class UProceduralMeshComponentPICO : public UProceduralMeshComponent
{
	GENERATED_BODY()
	
public:
	void SetUpdateTime(uint64 Time);
	int64 GetUpdateTime() const;
	TArray<int32> GetCachedIndices() { return CachedIndices; };
	bool IsEqualWithCached(TArray<int32>& Indices);
	void SetCachedIndices(const TArray<int32>& Indices) {CachedIndices = Indices; };
	
private:
	TMap<int32,ESemanticLabelPICO> IndexToSemanticLabelMap; 
	uint64 LastUpdateTime = 0;
	TArray<int32> CachedIndices;
};