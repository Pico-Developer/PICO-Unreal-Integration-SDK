// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/SceneCapture2D.h"
#include "PXR_MRCModule.h"
#include "PXR_MRCCastingCameraActor.generated.h"

class UPXRInGameThirdCamState;
class UTextureRenderTarget2D;
class UMaterial;
class UMaterialInstanceDynamic;

UCLASS(NotPlaceable, NotBlueprintable)
class APICOXRMRC_CastingCameraActor : public ASceneCapture2D
{
	GENERATED_BODY()
public:
	APICOXRMRC_CastingCameraActor(const FObjectInitializer& ObjectInitializer);

	/** Initialize the MRC settings and states */
	void InitializeStates(UPXRInGameThirdCamState * MRStateIn);

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void BeginDestroy() override;

	bool HasInitializedOnce() { return bHasInitializedInGameCamOnce; }

	UPROPERTY()
	ASceneCapture2D* ForegroundCaptureActor;

	UPROPERTY()
	UTextureRenderTarget2D* BackgroundRenderTarget;

	UPROPERTY()
	UTextureRenderTarget2D* ForegroundRenderTarget;

	bool bEnableForeground;
private:
	
	void InitializeInGameCam();
	void InitializeRTSize();
	void SetMRCTrackingReference();
	void UpdateInGameCamPose();
	void UpdateCamMatrixAndDepth();
	void SpawnForegroundCaptureActor();
	void DestroyForeroundCaptureActor();

	float ForegroundMaxDistance;
	bool bHasInitializedInGameCamOnce;

private:
	UPROPERTY()
	UPXRInGameThirdCamState* MRState;

	UPROPERTY()
	UMaterial* M_MRC;

	UPROPERTY()
	UMaterialInstanceDynamic * MI_Background;

	UPROPERTY()
	UMaterialInstanceDynamic*  MI_Foreground;
};
