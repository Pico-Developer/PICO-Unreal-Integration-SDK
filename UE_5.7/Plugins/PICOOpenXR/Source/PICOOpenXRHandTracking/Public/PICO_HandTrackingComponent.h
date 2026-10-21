// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PoseableMeshComponent.h"
#include "HeadMountedDisplayTypes.h"
#include "PICO_HandTrackingComponent.generated.h"

UENUM(BlueprintType)
enum class EHandTypePICO : uint8
{
	HandLeft,
	HandRight,
};

UCLASS(Blueprintable, ClassGroup = (PICO), meta = (BlueprintSpawnableComponent, DisplayName = "Hand Tracking Component PICO"))
class PICOOPENXRHANDTRACKING_API UHandTrackingComponentPICO : public UPoseableMeshComponent
{
	GENERATED_UCLASS_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/** Which type of the skeleton mesh, left hand or right hand. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PICO|HandTracking")
	EHandTypePICO SkeletonMeshType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PICO|HandTracking")
	bool bUseBuiltInHandMesh;

	/** By default only updates the position of the Wrist binding bone. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PICO|HandTracking")
	bool ApplyLocationToEveryBone;

	/** Whether to automatically hide this component based on the validity of the hand tracking data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PICO|HandTracking")
	bool AutoHide;

	/** Bone mapping for custom hand skeletal meshes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PICO|HandTracking")
	TMap<EHandKeypoint, FName> BoneMappings;

	/** Auto scale this hand component, default scale = 1.0f */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PICO|HandTracking")
	bool AutoScaleComponent;
private:
	bool bHandTrackingAvailable;
	bool bIsRunning;

	static int HandTrackingInstanceCount;
	static bool bHandTrackingStartedByComponent;
};
