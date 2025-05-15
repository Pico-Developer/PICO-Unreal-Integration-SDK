// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PXR_MRCState.generated.h"

struct FPXRTrackedCamera
{
	float FOV;
	int32 Width;
	int32 Height;
	FRotator CalibratedRotation;
	FVector CalibratedOffset;
	
	FPXRTrackedCamera()
		: FOV(90.0f)
		, Width(1280)
		, Height(720)
		, CalibratedRotation(EForceInit::ForceInitToZero)
		, CalibratedOffset(EForceInit::ForceInitToZero)
	{}
};

UCLASS()
class UPXRInGameThirdCamState : public UObject
{
	GENERATED_BODY()

public:

	UPXRInGameThirdCamState(const FObjectInitializer& ObjectInitializer);

	FPXRTrackedCamera TrackedCamera;

	FTransform FinalTransform;//Relative to the tracking reference

	float ZOffset;

	class USceneComponent* CurrentTrackingReference;

	bool bUseCustomTrans;

	FTransform CustomTrans;

	bool bUpdateMRCCameraZ;
};
