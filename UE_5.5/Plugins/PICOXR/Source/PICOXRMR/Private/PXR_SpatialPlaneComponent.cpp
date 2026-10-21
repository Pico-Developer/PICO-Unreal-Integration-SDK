// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_SpatialPlaneComponent.h"

UPICOSpatialPlaneComponent::UPICOSpatialPlaneComponent(const FObjectInitializer& ObjectInitializer):
	Super(ObjectInitializer), SpatialPlaneSemantic(EPICOSemanticLabel::Unknown), SpatialPlaneOrientation(EPICOPlaneOrientation::HorizontalUpward)
{
	PrimaryComponentTick.bCanEverTick = false;
}

EPICOSemanticLabel UPICOSpatialPlaneComponent::GetPlaneSemantic()
{
	return SpatialPlaneSemantic;
}

void UPICOSpatialPlaneComponent::SetPlaneSemantic(const EPICOSemanticLabel& SemanticLabel)
{
	SpatialPlaneSemantic = SemanticLabel;
}

EPICOPlaneOrientation UPICOSpatialPlaneComponent::GetPlaneOrientation() const
{
	return SpatialPlaneOrientation;
}

void UPICOSpatialPlaneComponent::SetPlaneOrientation(const EPICOPlaneOrientation& PlaneOrientation)
{
	SpatialPlaneOrientation =PlaneOrientation;
}


