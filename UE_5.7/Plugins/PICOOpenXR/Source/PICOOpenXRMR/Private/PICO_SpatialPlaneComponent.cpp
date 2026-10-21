// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.


#include "PICO_SpatialPlaneComponent.h"


USpatialPlaneComponentPICO::USpatialPlaneComponentPICO(const FObjectInitializer& ObjectInitializer):
	Super(ObjectInitializer), SpatialPlaneSemantic(ESemanticLabelPICO::Unknown), SpatialPlaneOrientation(EPlaneOrientationPICO::HorizontalUpward)
{
	PrimaryComponentTick.bCanEverTick = false;
}

ESemanticLabelPICO USpatialPlaneComponentPICO::GetPlaneSemantic()
{
	return SpatialPlaneSemantic;
}

void USpatialPlaneComponentPICO::SetPlaneSemantic(const ESemanticLabelPICO& SemanticLabel)
{
	SpatialPlaneSemantic = SemanticLabel;
}

EPlaneOrientationPICO USpatialPlaneComponentPICO::GetPlaneOrientation() const
{
	return SpatialPlaneOrientation;
}

void USpatialPlaneComponentPICO::SetPlaneOrientation(const EPlaneOrientationPICO& PlaneOrientation)
{
	SpatialPlaneOrientation =PlaneOrientation;
}

