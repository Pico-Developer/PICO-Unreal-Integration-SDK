#pragma once

#include "CoreMinimal.h"
#include "HeadMountedDisplayTypes.h"
#include "ProceduralMeshComponent.h"
#include "PICO_OpenXRRenderModelPMC.generated.h"

UCLASS(ClassGroup = (PICO), meta = (BlueprintSpawnableComponent))
class PICOOPENXRHMD_API UOpenXRRenderModelPMCPICO : public UProceduralMeshComponent
{
	GENERATED_BODY()

public:
	UOpenXRRenderModelPMCPICO(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|HMD|RenderModel")
	EControllerHand Hand = EControllerHand::Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|HMD|RenderModel")
	class UMaterialInterface* OverrideMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|HMD|RenderModel")
	bool bAutoReload = true;

	UFUNCTION(BlueprintCallable, Category = "PICO|HMD|RenderModel")
	bool TryLoadRenderModel();

protected:
	virtual void BeginPlay() override;

private:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	bool LoadFromOpenXRRenderModel(EControllerHand InHand);
	bool LoadFromOpenXRRenderModelPICO();
	bool bMeshLoaded = false;
	int32 LoadAttempts = 0;
	int32 RetryEveryNFrames = 20;
	int32 FramesSinceLastAttempt = 0;

	virtual void OnRegister() override;
	virtual void OnUnregister() override;
};
