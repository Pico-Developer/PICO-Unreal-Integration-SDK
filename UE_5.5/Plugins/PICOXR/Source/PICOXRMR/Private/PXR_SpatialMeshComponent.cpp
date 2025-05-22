// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_SpatialMeshComponent.h"
#include "PXR_MRTypes.h"

UPICOSpatialMeshComponent::UPICOSpatialMeshComponent(const FObjectInitializer& ObjectInitializer):
	Super(ObjectInitializer)
{
}

EPICOSemanticLabel UPICOSpatialMeshComponent::GetSemanticByIndex(int32 Index)
{
	return IndexToAnchorSceneLabelMap.Find(Index)?IndexToAnchorSceneLabelMap[Index]:EPICOSemanticLabel::Unknown;
}

void UPICOSpatialMeshComponent::AddIndexToSemanticLabel(int32 Index, EPICOSemanticLabel SceneLabel)
{
	IndexToAnchorSceneLabelMap.Emplace(Index,SceneLabel);
}

void UPICOSpatialMeshComponent::SetUpdateTime(uint64 Time)
{
	LastUpdateTime=Time;
}

int64 UPICOSpatialMeshComponent::GetUpdateTime() const
{
	return static_cast<int64>(LastUpdateTime);
}

bool UPICOSpatialMeshComponent::IsEqualWithCached(TArray<int32>& Indices)
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
