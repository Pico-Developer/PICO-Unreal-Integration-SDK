// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_SpatialMeshComponent.h"
#include "PICO_MRTypes.h"

USpatialMeshComponentPICO::USpatialMeshComponentPICO(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

ESemanticLabelPICO USpatialMeshComponentPICO::GetSemanticByIndex(int32 Index)
{
	return IndexToSemanticLabelMap.Find(Index)?IndexToSemanticLabelMap[Index]:ESemanticLabelPICO::Unknown;
}

void USpatialMeshComponentPICO::AddIndexToSemanticLabel(int32 Index, ESemanticLabelPICO SceneLabel)
{
	IndexToSemanticLabelMap.Emplace(Index,SceneLabel);
}