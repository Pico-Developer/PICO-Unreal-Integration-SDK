// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "PICO_MovementFunctionLibrary.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "PICO_FaceTrackingComponent.generated.h"

struct PICOOPENXRMOVEMENT_API FMorphTargetsManagerPICO
{
public:
	void ResetMeshMorphTargetCurves(USkinnedMeshComponent* TargetMeshComponent);
	void UpdateMeshMorphTargets(USkinnedMeshComponent* TargetMeshComponent);
	void SetMeshMorphTargetValue(FName MorphTargetName, float Value);
	float GetMeshMorphTargetValue(FName MorphTargetName) const;
	void EmptyMorphTargets();
	TMap<FName, float> MeshMorphTargetCurves;
};

UCLASS(Blueprintable, ClassGroup = (PICO), meta = (BlueprintSpawnableComponent, DisplayName = "Face Tracking Component PICO"))
class PICOOPENXRMOVEMENT_API UFaceTrackingComponentPICO : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFaceTrackingComponentPICO();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "PICO|FaceTracking", meta = (UnsafeDuringActorConstruction = "true"))
		void SetBlendShapeValue(EFaceBlendShapePICO BlendShape, float Value);

	UFUNCTION(BlueprintCallable, Category = "PICO|FaceTracking")
		float GetBlendShapeValue(EFaceBlendShapePICO BlendShape) const;

	UFUNCTION(BlueprintCallable, Category = "PICO|FaceTracking")
		void ClearBlendShapeValues();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|FaceTracking")
		FName FTTargetMeshComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|FaceTracking")
		float InvalidFaceDataResetTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|FaceTracking")
		TMap<EFaceBlendShapePICO, FName> BlendShapeNameMapping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|FaceTracking")
		bool bUpdateFaceTracking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|FaceTracking")
		EFaceTrackingModePICO Mode;
private:
	bool InitializeFaceTracking();

	UPROPERTY()
		USkinnedMeshComponent* FTTargetMeshComponent;

	TStaticArray<bool, static_cast<uint32>(EFaceBlendShapePICO::COUNT)> ValidBlendShape;

	FMorphTargetsManagerPICO MorphTargetsManager;

	float InvalidFaceDataTimer;

	bool bFaceTrackingStarted = false;

};
