// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#include "PXR_SpatialMeshComponent.h"


UPICOSpatialMeshComponent::UPICOSpatialMeshComponent(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

EPICOSemanticLabel UPICOSpatialMeshComponent::GetSemanticByIndex(int32 Index)
{
	return IndexToSemanticLabelMap.Find(Index)?IndexToSemanticLabelMap[Index]:EPICOSemanticLabel::Unknown;
}

void UPICOSpatialMeshComponent::AddIndexToSemanticLabel(int32 Index, EPICOSemanticLabel SceneLabel)
{
	IndexToSemanticLabelMap.Emplace(Index,SceneLabel);
}

