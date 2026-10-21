// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_HandTrackingComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Engine/SkeletalMesh.h"
#include "PICO_HandTrackingFunctionLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

int UHandTrackingComponentPICO::HandTrackingInstanceCount = 0;
bool UHandTrackingComponentPICO::bHandTrackingStartedByComponent = false;

namespace
{
	USkeletalMesh* GetBuiltInHandMesh(EHandTypePICO HandType)
	{
		static TWeakObjectPtr<USkeletalMesh> LeftMesh;
		static TWeakObjectPtr<USkeletalMesh> RightMesh;

		TWeakObjectPtr<USkeletalMesh>& Cache = (HandType == EHandTypePICO::HandRight) ? RightMesh : LeftMesh;
		if (!Cache.IsValid())
		{
			const TCHAR* Path = (HandType == EHandTypePICO::HandRight)
				? TEXT("/PICOOpenXR/Meshes/Hand/Right/SM_Hand_R.SM_Hand_R")
				: TEXT("/PICOOpenXR/Meshes/Hand/Left/SM_Hand_L.SM_Hand_L");

			Cache = LoadObject<USkeletalMesh>(nullptr, Path);
		}

		return Cache.Get();
	}

	bool IsBuiltInHandMeshAsset(const USkinnedAsset* SkinnedAsset)
	{
		if (SkinnedAsset == nullptr)
		{
			return false;
		}

		return SkinnedAsset == GetBuiltInHandMesh(EHandTypePICO::HandLeft) ||
			SkinnedAsset == GetBuiltInHandMesh(EHandTypePICO::HandRight);
	}

	void ApplyBuiltInBoneMappings(EHandTypePICO HandType, TMap<EHandKeypoint, FName>& InOutMappings)
	{
		static const TCHAR* LeftBoneNames[EHandKeypointCount] = {
			TEXT("left_palm"),
			TEXT("left_wrist"),
			TEXT("left_thumb_metacarpal"),
			TEXT("left_thumb_proximal"),
			TEXT("left_thumb_distal"),
			TEXT("left_thumb_tip"),
			TEXT("left_index_metacarpal"),
			TEXT("left_index_proximal"),
			TEXT("left_index_intermediate"),
			TEXT("left_index_distal"),
			TEXT("left_index_tip"),
			TEXT("left_middle_metacarpal"),
			TEXT("left_middle_proximal"),
			TEXT("left_middle_intermediate"),
			TEXT("left_middle_distal"),
			TEXT("left_middle_tip"),
			TEXT("left_ring_metacarpal"),
			TEXT("left_ring_proximal"),
			TEXT("left_ring_intermediate"),
			TEXT("left_ring_distal"),
			TEXT("left_ring_tip"),
			TEXT("left_little_metacarpal"),
			TEXT("left_little_proximal"),
			TEXT("left_little_intermediate"),
			TEXT("left_little_distal"),
			TEXT("left_little_tip")
		};

		static const TCHAR* RightBoneNames[EHandKeypointCount] = {
			TEXT("right_palm"),
			TEXT("right_wrist"),
			TEXT("right_thumb_metacarpal"),
			TEXT("right_thumb_proximal"),
			TEXT("right_thumb_distal"),
			TEXT("right_thumb_tip"),
			TEXT("right_index_metacarpal"),
			TEXT("right_index_proximal"),
			TEXT("right_index_intermediate"),
			TEXT("right_index_distal"),
			TEXT("right_index_tip"),
			TEXT("right_middle_metacarpal"),
			TEXT("right_middle_proximal"),
			TEXT("right_middle_intermediate"),
			TEXT("right_middle_distal"),
			TEXT("right_middle_tip"),
			TEXT("right_ring_metacarpal"),
			TEXT("right_ring_proximal"),
			TEXT("right_ring_intermediate"),
			TEXT("right_ring_distal"),
			TEXT("right_ring_tip"),
			TEXT("right_little_metacarpal"),
			TEXT("right_little_proximal"),
			TEXT("right_little_intermediate"),
			TEXT("right_little_distal"),
			TEXT("right_little_tip")
		};

		const bool bRight = HandType == EHandTypePICO::HandRight;
		const TCHAR* const* DesiredBoneNames = bRight ? RightBoneNames : LeftBoneNames;

		for (int32 KeypointIndex = 0; KeypointIndex < EHandKeypointCount; ++KeypointIndex)
		{
			const EHandKeypoint Key = static_cast<EHandKeypoint>(KeypointIndex);
			FName& Existing = InOutMappings.FindOrAdd(Key);
			const FName LeftName(LeftBoneNames[KeypointIndex]);
			const FName RightName(RightBoneNames[KeypointIndex]);
			if (Existing.IsNone() || Existing == LeftName || Existing == RightName)
			{
				Existing = FName(DesiredBoneNames[KeypointIndex]);
			}
		}
	}

	void ClearBuiltInBoneMappings(TMap<EHandKeypoint, FName>& InOutMappings)
	{
		static const TCHAR* LeftBoneNames[EHandKeypointCount] = {
			TEXT("left_palm"),
			TEXT("left_wrist"),
			TEXT("left_thumb_metacarpal"),
			TEXT("left_thumb_proximal"),
			TEXT("left_thumb_distal"),
			TEXT("left_thumb_tip"),
			TEXT("left_index_metacarpal"),
			TEXT("left_index_proximal"),
			TEXT("left_index_intermediate"),
			TEXT("left_index_distal"),
			TEXT("left_index_tip"),
			TEXT("left_middle_metacarpal"),
			TEXT("left_middle_proximal"),
			TEXT("left_middle_intermediate"),
			TEXT("left_middle_distal"),
			TEXT("left_middle_tip"),
			TEXT("left_ring_metacarpal"),
			TEXT("left_ring_proximal"),
			TEXT("left_ring_intermediate"),
			TEXT("left_ring_distal"),
			TEXT("left_ring_tip"),
			TEXT("left_little_metacarpal"),
			TEXT("left_little_proximal"),
			TEXT("left_little_intermediate"),
			TEXT("left_little_distal"),
			TEXT("left_little_tip")
		};

		static const TCHAR* RightBoneNames[EHandKeypointCount] = {
			TEXT("right_palm"),
			TEXT("right_wrist"),
			TEXT("right_thumb_metacarpal"),
			TEXT("right_thumb_proximal"),
			TEXT("right_thumb_distal"),
			TEXT("right_thumb_tip"),
			TEXT("right_index_metacarpal"),
			TEXT("right_index_proximal"),
			TEXT("right_index_intermediate"),
			TEXT("right_index_distal"),
			TEXT("right_index_tip"),
			TEXT("right_middle_metacarpal"),
			TEXT("right_middle_proximal"),
			TEXT("right_middle_intermediate"),
			TEXT("right_middle_distal"),
			TEXT("right_middle_tip"),
			TEXT("right_ring_metacarpal"),
			TEXT("right_ring_proximal"),
			TEXT("right_ring_intermediate"),
			TEXT("right_ring_distal"),
			TEXT("right_ring_tip"),
			TEXT("right_little_metacarpal"),
			TEXT("right_little_proximal"),
			TEXT("right_little_intermediate"),
			TEXT("right_little_distal"),
			TEXT("right_little_tip")
		};

		for (int32 KeypointIndex = 0; KeypointIndex < EHandKeypointCount; ++KeypointIndex)
		{
			const EHandKeypoint Key = static_cast<EHandKeypoint>(KeypointIndex);
			if (FName* Existing = InOutMappings.Find(Key))
			{
				const FName LeftName(LeftBoneNames[KeypointIndex]);
				const FName RightName(RightBoneNames[KeypointIndex]);
				if (*Existing == LeftName || *Existing == RightName)
				{
					*Existing = NAME_None;
				}
			}
		}
	}
}

UHandTrackingComponentPICO::UHandTrackingComponentPICO(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
	, SkeletonMeshType(EHandTypePICO::HandLeft)
	, bUseBuiltInHandMesh(true)
	, ApplyLocationToEveryBone(false)
	, AutoHide(false)
	, bHandTrackingAvailable(false)
	, bIsRunning(false)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	for (int KeypointIndex = 0; KeypointIndex < EHandKeypointCount; KeypointIndex++)
	{
		BoneMappings.Add(EHandKeypoint(KeypointIndex), NAME_None);
	}

	if (bUseBuiltInHandMesh)
	{
		ApplyBuiltInBoneMappings(SkeletonMeshType, BoneMappings);

		if (GetSkinnedAsset() == nullptr)
		{
			static ConstructorHelpers::FObjectFinder<USkeletalMesh> LeftMeshFinder(TEXT("/PICOOpenXR/Meshes/Hand/Left/SM_Hand_L.SM_Hand_L"));
			static ConstructorHelpers::FObjectFinder<USkeletalMesh> RightMeshFinder(TEXT("/PICOOpenXR/Meshes/Hand/Right/SM_Hand_R.SM_Hand_R"));

			USkeletalMesh* DesiredMesh = (SkeletonMeshType == EHandTypePICO::HandRight)
				? (RightMeshFinder.Succeeded() ? RightMeshFinder.Object : nullptr)
				: (LeftMeshFinder.Succeeded() ? LeftMeshFinder.Object : nullptr);

			if (DesiredMesh != nullptr)
			{
				SetSkinnedAsset(DesiredMesh);
			}
		}
	}
}

void UHandTrackingComponentPICO::OnRegister()
{
	Super::OnRegister();

	if (!bUseBuiltInHandMesh)
	{
		return;
	}

	USkinnedAsset* CurrentAsset = GetSkinnedAsset();
	USkeletalMesh* DesiredMesh = GetBuiltInHandMesh(SkeletonMeshType);
	if (DesiredMesh != nullptr && (CurrentAsset == nullptr || IsBuiltInHandMeshAsset(CurrentAsset)))
	{
		SetSkinnedAssetAndUpdate(DesiredMesh);
	}

	ApplyBuiltInBoneMappings(SkeletonMeshType, BoneMappings);
}

#if WITH_EDITOR
void UHandTrackingComponentPICO::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName != GET_MEMBER_NAME_CHECKED(UHandTrackingComponentPICO, bUseBuiltInHandMesh) &&
		PropertyName != GET_MEMBER_NAME_CHECKED(UHandTrackingComponentPICO, SkeletonMeshType))
	{
		return;
	}

	if (bUseBuiltInHandMesh)
	{
		USkinnedAsset* CurrentAsset = GetSkinnedAsset();
		USkeletalMesh* DesiredMesh = GetBuiltInHandMesh(SkeletonMeshType);
		if (DesiredMesh != nullptr && (CurrentAsset == nullptr || IsBuiltInHandMeshAsset(CurrentAsset)))
		{
			SetSkinnedAssetAndUpdate(DesiredMesh);
		}
		ApplyBuiltInBoneMappings(SkeletonMeshType, BoneMappings);
	}
	else
	{
		if (IsBuiltInHandMeshAsset(GetSkinnedAsset()))
		{
			SetSkinnedAssetAndUpdate(nullptr);
		}
		ClearBuiltInBoneMappings(BoneMappings);
	}

	MarkRenderStateDirty();
}
#endif

void UHandTrackingComponentPICO::BeginPlay()
{
	Super::BeginPlay();
	FXRMotionControllerData Data;
	UHeadMountedDisplayFunctionLibrary::GetMotionControllerData(nullptr, EControllerHand(SkeletonMeshType), Data);
	if (Data.DeviceVisualType == EXRVisualType::Hand || UHandTrackingFunctionLibraryPICO::IsHandTrackingSupportPICO())
	{
		bHandTrackingAvailable = true;
	}
	if (AutoHide)
	{
		SetHiddenInGame(true, true);
	}
	++HandTrackingInstanceCount;
}

void UHandTrackingComponentPICO::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	HandTrackingInstanceCount = FMath::Max(HandTrackingInstanceCount - 1, 0);
	if (HandTrackingInstanceCount == 0 && bHandTrackingStartedByComponent)
	{
		UHandTrackingFunctionLibraryPICO::StopHandTrackingPICO();
		bHandTrackingStartedByComponent = false;
	}
}

void UHandTrackingComponentPICO::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	bool bHidden = true;
	if (bHandTrackingAvailable && GetSkinnedAsset())
	{
		FXRMotionControllerData Data;
		UHeadMountedDisplayFunctionLibrary::GetMotionControllerData(nullptr, EControllerHand(SkeletonMeshType), Data);
		if (Data.bValid)
		{
			bHidden = false;
			for (auto& BoneMapping : BoneMappings)
			{
				const auto& HandKeypoint = BoneMapping.Key;
				const auto& BoneName = BoneMapping.Value;
				int32 BoneIndex = GetSkinnedAsset()->GetRefSkeleton().FindBoneIndex(BoneName);
				if (BoneIndex >= 0)
				{
					if (Data.HandKeyPositions.IsValidIndex((uint8)HandKeypoint))
					{
						const FQuat& WorldRotation = Data.HandKeyRotations[(uint8)HandKeypoint];
						const FVector& WorldLocation = Data.HandKeyPositions[(uint8)HandKeypoint];

						SetBoneRotationByName(BoneName, WorldRotation.Rotator(), EBoneSpaces::WorldSpace);

						if (HandKeypoint == EHandKeypoint::Wrist || ApplyLocationToEveryBone)
						{
							SetBoneLocationByName(BoneName, WorldLocation, EBoneSpaces::WorldSpace);
						}

						if (HandKeypoint == EHandKeypoint::Wrist)
						{
							this->SetWorldLocation(WorldLocation);
							this->SetWorldRotation(WorldRotation);
						}

						if (AutoScaleComponent)
						{
							float Scale = 1.0f;
							UHandTrackingFunctionLibraryPICO::GetHandTrackingMeshScalePICO(EControllerHand(SkeletonMeshType), Scale);
							this->SetRelativeScale3D(FVector(Scale));
						}
					}
				}
			}
		}
		else
		{
			if (!bIsRunning)
			{
				if (UHandTrackingFunctionLibraryPICO::StartHandTrackingPICO())
				{
					bHandTrackingStartedByComponent = true;
				}
			}

			bIsRunning = UHandTrackingFunctionLibraryPICO::IsHandTrackingRunningPICO();

			// Fallback: use UHandTrackingFunctionLibraryPICO when GetMotionControllerData returns invalid data
			TArray<FVector> OutPositions;
			TArray<FQuat> OutRotations;
			TArray<float> OutRadii;
			TArray<FVector> LinearVelocity;
			TArray<FVector> AngularVelocity;
			float Scale = 1.0f;
			if (bIsRunning && UHandTrackingFunctionLibraryPICO::UpdateHandTrackingDataPICO() && UHandTrackingFunctionLibraryPICO::GetHandTrackingDataPICO(EControllerHand(SkeletonMeshType), OutPositions, OutRotations, OutRadii, LinearVelocity, AngularVelocity, Scale))
			{
				bHidden = false;
				for (auto& BoneMapping : BoneMappings)
				{
					const auto& HandKeypoint = BoneMapping.Key;
					const auto& BoneName = BoneMapping.Value;
					int32 BoneIndex = GetSkinnedAsset()->GetRefSkeleton().FindBoneIndex(BoneName);
					if (BoneIndex >= 0)
					{
						uint8 KeypointIndex = (uint8)HandKeypoint;
						if (OutPositions.IsValidIndex(KeypointIndex) && OutRotations.IsValidIndex(KeypointIndex))
						{
							const FQuat& WorldRotation = OutRotations[KeypointIndex];
							const FVector& WorldLocation = OutPositions[KeypointIndex];

							SetBoneRotationByName(BoneName, WorldRotation.Rotator(), EBoneSpaces::WorldSpace);

							if (HandKeypoint == EHandKeypoint::Wrist || ApplyLocationToEveryBone)
							{
								SetBoneLocationByName(BoneName, WorldLocation, EBoneSpaces::WorldSpace);
							}

							if (HandKeypoint == EHandKeypoint::Wrist)
							{
								this->SetWorldLocation(WorldLocation);
								this->SetWorldRotation(WorldRotation);
							}

							if (AutoScaleComponent)
							{
								this->SetRelativeScale3D(FVector(Scale));
							}
						}
					}
				}
			}
		}
	}

	if (AutoHide && bHidden != bHiddenInGame)
	{
		SetHiddenInGame(bHidden, true);
	}
}
