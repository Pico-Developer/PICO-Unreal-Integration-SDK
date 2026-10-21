// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "PICO_MRCCamera.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup = (PICO), meta = (BlueprintSpawnableComponent))
class PICOOPENXRHMD_API AMRCCameraPICO : public ASceneCapture2D
{
	GENERATED_BODY()
	
public:
	AMRCCameraPICO(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
	class UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY()
	UTextureRenderTarget2D* BackgroundRT;

	UPROPERTY()
	UTextureRenderTarget2D* ForegroundRT;

	bool DisableForegroundLayer = false;

private:
	class FHMDPICO* HMDPICO;
	class IXRTrackingSystem* XRTrackingSystem = nullptr;
	int Width;
	int Height;
	float Fov;
	bool FlipFlop;
	void InitializeRTRenderResource(UTextureRenderTarget2D* RT);
	void UpdateCamMatrixAndDepth(bool bIsForeground);
};
