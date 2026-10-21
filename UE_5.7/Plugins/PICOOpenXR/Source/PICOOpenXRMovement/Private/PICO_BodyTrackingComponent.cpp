// Fill out your copyright notice in the Description page of Project Settings.


#include "PICO_BodyTrackingComponent.h"
#include "Engine/SkinnedAsset.h"
#include "PICO_MovementUtility.h"

int UBodyTrackingComponentPICO::TrackingInstanceCount = 0;

UBodyTrackingComponentPICO::UBodyTrackingComponentPICO()
	: BodyTrackingMode(EBodyTrackingModePICO::HighAccuracy)
	, OnlyUseRotation(true)
	, WorldToMeters(100.f)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// Setup defaults
	BoneNames.Add(EBodyJointPICO::Pelvis, "Pelvis");
	BoneNames.Add(EBodyJointPICO::LeftHip, "LeftHip");
	BoneNames.Add(EBodyJointPICO::RightHip, "RightHip");
	BoneNames.Add(EBodyJointPICO::Spine1, "Spine1");
	BoneNames.Add(EBodyJointPICO::LeftKnee, "LeftKnee");
	BoneNames.Add(EBodyJointPICO::RightKnee, "RightKnee");
	BoneNames.Add(EBodyJointPICO::Spine2, "Spine2");
	BoneNames.Add(EBodyJointPICO::LeftAnkle, "LeftAnkle");
	BoneNames.Add(EBodyJointPICO::RightAnkle, "RightAnkle");
	BoneNames.Add(EBodyJointPICO::Spine3, "Spine3");
	BoneNames.Add(EBodyJointPICO::LeftFoot, "LeftFoot");
	BoneNames.Add(EBodyJointPICO::RightFoot, "RightFoot");
	BoneNames.Add(EBodyJointPICO::Neck, "Neck");
	BoneNames.Add(EBodyJointPICO::LeftCollar, "LeftCollar");
	BoneNames.Add(EBodyJointPICO::RightCollar, "RightCollar");
	BoneNames.Add(EBodyJointPICO::Head, "Head");
	BoneNames.Add(EBodyJointPICO::LeftShoulder, "LeftShoulder");
	BoneNames.Add(EBodyJointPICO::RightShoulder, "RightShoulder");
	BoneNames.Add(EBodyJointPICO::LeftElbow, "LeftElbow");
	BoneNames.Add(EBodyJointPICO::RightElbow, "RightElbow");
	BoneNames.Add(EBodyJointPICO::LeftWrist, "LeftWrist");
	BoneNames.Add(EBodyJointPICO::RightWrist, "RightWrist");
	BoneNames.Add(EBodyJointPICO::LeftHand, "LeftHand");
	BoneNames.Add(EBodyJointPICO::RightHand, "RightHand");
}

void UBodyTrackingComponentPICO::BeginPlay()
{
	Super::BeginPlay();

	if (!UMovementFunctionLibraryPICO::IsBodyTrackingSupportedPICO())
	{
		// Early exit if body tracking isn't supported
		UE_LOG(PICOOpenXRMovement, Warning, TEXT("Body tracking is not supported. (%s:%s)"), *GetOwner()->GetName(), *GetName());
		SetComponentTickEnabled(false);
		return;
	}

	if (!GetUnitScaleFactorFromSettingsPICO(GetWorld(), WorldToMeters))
	{
		UE_LOG(PICOOpenXRMovement, Warning, TEXT("Cannot get world settings. (%s:%s)"), *GetOwner()->GetName(), *GetName());
	}

	if (!InitializeBodyBones())
	{
		UE_LOG(PICOOpenXRMovement, Warning, TEXT("Failed to initialize body data. (%s: %s)"), *GetOwner()->GetName(), *GetName());
		SetComponentTickEnabled(false);
		return;
	}

	if (!UMovementFunctionLibraryPICO::StartBodyTrackingPICO(BodyTrackingMode))
	{
		UE_LOG(PICOOpenXRMovement, Warning, TEXT("Failed to start body tracking. (%s: %s)"), *GetOwner()->GetName(), *GetName());
		SetComponentTickEnabled(false);
		return;
	}
	++TrackingInstanceCount;
}

void UBodyTrackingComponentPICO::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsComponentTickEnabled())
	{
		if (--TrackingInstanceCount == 0)
		{
			if (!UMovementFunctionLibraryPICO::StopBodyTrackingPICO())
			{
				UE_LOG(PICOOpenXRMovement, Warning, TEXT("Failed to stop body tracking. (%s: %s)"), *GetOwner()->GetName(), *GetName());
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UBodyTrackingComponentPICO::ResetAllBoneTransforms()
{
	for (int i = 0; i < BodyState.BaseJointsData.Num(); ++i)
	{
		int32* BoneIndex = MappedBoneIndices.Find(static_cast<EBodyJointPICO>(i));
		if (BoneIndex != nullptr)
		{
			ResetBoneTransformByName(BoneNames[static_cast<EBodyJointPICO>(i)]);
		}
	}
}

bool UBodyTrackingComponentPICO::InitializeBodyBones()
{
	if (GetSkinnedAsset() == nullptr)
	{
		UE_LOG(PICOOpenXRMovement, Display, TEXT("No SkeletalMesh in this component."));
		return false;
	}

	for (const auto& it : BoneNames)
	{
		int32 BoneIndex = GetBoneIndex(it.Value);

		if (BoneIndex == INDEX_NONE)
		{
			UE_LOG(PICOOpenXRMovement, Display, TEXT("Could not find bone %s in skeletal mesh %s"), *StaticEnum<EBodyJointPICO>()->GetValueAsString(it.Key), *GetSkinnedAsset()->GetName());
		}
		else
		{
			MappedBoneIndices.Add(it.Key, BoneIndex);
		}
	}

	return true;
}

void UBodyTrackingComponentPICO::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (UMovementFunctionLibraryPICO::TryGetBodyStatePICO(BodyState, WorldToMeters))
	{
		if (BodyState.IsActive)
		{
			for (int i = 0; i < BodyState.BaseJointsData.Num(); ++i)
			{
				const FVector& Position = BodyState.BaseJointsData[i].Position;
				const FRotator& Orientation = BodyState.BaseJointsData[i].Orientation;

				int32* BoneIndex = MappedBoneIndices.Find(static_cast<EBodyJointPICO>(i));
				if (BoneIndex != nullptr)
				{
					const FName& BoneName = BoneNames[static_cast<EBodyJointPICO>(i)];
					SetBoneRotationByName(BoneName, Orientation, EBoneSpaces::ComponentSpace);
					if (!OnlyUseRotation)
					{
						SetBoneLocationByName(BoneName, Position, EBoneSpaces::ComponentSpace);
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(PICOOpenXRMovement, Verbose, TEXT("Failed to get body state (%s:%s)."), *GetOwner()->GetName(), *GetName());
	}
}
