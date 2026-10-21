// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PoseableMeshComponent.h"
#include "PICO_MovementFunctionLibrary.h"
#include "PICO_BodyTrackingComponent.generated.h"

UCLASS(Blueprintable, ClassGroup = (PICO), meta = (BlueprintSpawnableComponent, DisplayName = "Body Tracking Component PICO"))
class PICOOPENXRMOVEMENT_API UBodyTrackingComponentPICO : public UPoseableMeshComponent
{
	GENERATED_BODY()
public:
	UBodyTrackingComponentPICO();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	* Restore all bones to their initial transforms
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|BodyTracking")
		void ResetAllBoneTransforms();

	/**
	* How are the results of body tracking applied to the mesh.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|BodyTracking")
		EBodyTrackingModePICO BodyTrackingMode;

	/**
	* Whether to use only rotation to drive the bone without position.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|BodyTracking")
		bool OnlyUseRotation;

	/**
	 * The bone name associated with each bone ID.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|BodyTracking")
		TMap<EBodyJointPICO, FName> BoneNames;

private:
	bool InitializeBodyBones();

	// One meter in unreal world units.
	float WorldToMeters;

	// The index of each mapped bone after the discovery and association of bone names.
	TMap<EBodyJointPICO, int32> MappedBoneIndices;

	// Saved body state.
	FBodyStatePICO BodyState;

	// Stop the tracker just once.
	static int TrackingInstanceCount;
};
