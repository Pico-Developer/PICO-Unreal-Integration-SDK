// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_ProceduralMeshComponent.h"

void UPICOProceduralMeshComponent::SetUpdateTime(uint64 Time)
{
	LastUpdateTime=Time;
}

int64 UPICOProceduralMeshComponent::GetUpdateTime() const
{
	return static_cast<int64>(LastUpdateTime);
}

bool UPICOProceduralMeshComponent::IsEqualWithCached(TArray<int32>& Indices)
{
	if (CachedIndices.Num() != Indices.Num())
	{
		return false;
	}

	for (int32 i = 0; i < CachedIndices.Num(); ++i)
	{
		if (CachedIndices[i] != Indices[i])
		{
			return false;
		}
	}

	return true;
}
