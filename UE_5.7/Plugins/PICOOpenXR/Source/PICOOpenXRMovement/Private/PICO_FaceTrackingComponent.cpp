// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_FaceTrackingComponent.h"
#include "AnimationRuntime.h"
#include "Engine/SkeletalMesh.h"
#include "PICO_MovementUtility.h"

// Sets default values for this component's properties
UFaceTrackingComponentPICO::UFaceTrackingComponentPICO()
	: FTTargetMeshComponentName(NAME_None)
	, InvalidFaceDataResetTime(2.0f)
	, bUpdateFaceTracking(true)
	, Mode(EFaceTrackingModePICO::Default)
	, FTTargetMeshComponent(nullptr)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	BlendShapeNameMapping.Add(EFaceBlendShapePICO::JawOpen, "jawOpen");
	// ...
}

// Called when the game starts
void UFaceTrackingComponentPICO::BeginPlay()
{
	Super::BeginPlay();

	TArray<EFaceTrackingModePICO> Modes;
	bool Supported = UMovementFunctionLibraryPICO::GetFaceTrackingSupportedPICO(Modes);
	if (!Supported)
	{
		UE_LOG(PICOOpenXRMovement, Warning, TEXT("Face tracking is not supported. (%s:%s)"), *GetOwner()->GetName(), *GetName());
		SetComponentTickEnabled(false);
		return;
	}

	if (FTTargetMeshComponentName == NAME_None)
	{
		UE_LOG(PICOOpenXRMovement, Warning, TEXT("Invalid mesh component name. (%s:%s)"), *GetOwner()->GetName(), *GetName());
		SetComponentTickEnabled(false);
		return;
	}

	if (!InitializeFaceTracking())
	{
		UE_LOG(PICOOpenXRMovement, Warning, TEXT("Failed to initialize face tracking. (%s:%s)"), *GetOwner()->GetName(), *GetName());
		SetComponentTickEnabled(false);
		return;
	}

	if (!UMovementFunctionLibraryPICO::StartFaceTrackingPICO(Mode))
	{
		UE_LOG(PICOOpenXRMovement, Warning, TEXT("Failed to start face tracking. (%s: %s)"), *GetOwner()->GetName(), *GetName());
		SetComponentTickEnabled(false);
		return;
	}
	bFaceTrackingStarted = true;
}

// Called every frame
void UFaceTrackingComponentPICO::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(FTTargetMeshComponent))
	{
		UE_LOG(PICOOpenXRMovement, Warning, TEXT("No target mesh specified. (%s:%s)"), *GetOwner()->GetName(), *GetName());
		return;
	}

	FFaceStatePICO FaceState;

	if (bUpdateFaceTracking && UMovementFunctionLibraryPICO::GetFaceTrackingDataPICO(0, FaceState) && (FaceState.IsUpperFaceDataValid || FaceState.IsLowerFaceDataValid))
	{
		InvalidFaceDataTimer = 0.0f;

		MorphTargetsManager.ResetMeshMorphTargetCurves(FTTargetMeshComponent);

		for (int32 FaceBlendShapeIndex = 0; FaceBlendShapeIndex < static_cast<int32>(XR_FACE_EXPRESSION_COUNT_BD); ++FaceBlendShapeIndex)
		{
			if (ValidBlendShape[FaceBlendShapeIndex])
			{
				FName BlendShapeName = BlendShapeNameMapping[static_cast<EFaceBlendShapePICO>(FaceBlendShapeIndex)];
				MorphTargetsManager.SetMeshMorphTargetValue(BlendShapeName, FaceState.FaceExpressionWeights[FaceBlendShapeIndex]);
			}
		}

		for (int32 FaceLipsyncBlendShapeIndex = XR_FACE_EXPRESSION_COUNT_BD; FaceLipsyncBlendShapeIndex < static_cast<int32>(XR_FACE_EXPRESSION_COUNT_BD + XR_LIP_EXPRESSION_COUNT_BD); ++FaceLipsyncBlendShapeIndex)
		{
			if (ValidBlendShape[FaceLipsyncBlendShapeIndex])
			{
				FName BlendShapeName = BlendShapeNameMapping[static_cast<EFaceBlendShapePICO>(FaceLipsyncBlendShapeIndex)];
				MorphTargetsManager.SetMeshMorphTargetValue(BlendShapeName, FaceState.LipsyncExpressionWeights[FaceLipsyncBlendShapeIndex - XR_FACE_EXPRESSION_COUNT_BD]);
			}
		}
	}
	else
	{
		InvalidFaceDataTimer += DeltaTime;
		if (InvalidFaceDataTimer >= InvalidFaceDataResetTime)
		{
			MorphTargetsManager.ResetMeshMorphTargetCurves(FTTargetMeshComponent);
		}
	}

	MorphTargetsManager.UpdateMeshMorphTargets(FTTargetMeshComponent);
}

void UFaceTrackingComponentPICO::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bFaceTrackingStarted)
	{
		if (!UMovementFunctionLibraryPICO::StopFaceTrackingPICO())
		{
			UE_LOG(PICOOpenXRMovement, Warning, TEXT("Failed to stop face tracking. (%s: %s)"), *GetOwner()->GetName(), *GetName());
		}
		bFaceTrackingStarted = false;
	}

	Super::EndPlay(EndPlayReason);
}

void UFaceTrackingComponentPICO::SetBlendShapeValue(EFaceBlendShapePICO BlendShape, float Value)
{
	if (BlendShape >= EFaceBlendShapePICO::COUNT)
	{
		UE_LOG(PICOOpenXRMovement, Warning, TEXT("Cannot set BlendShape value with invalid BlendShape index."));
		return;
	}

	if (!ValidBlendShape[static_cast<int32>(BlendShape)])
	{
		UE_LOG(PICOOpenXRMovement, Warning, TEXT("Cannot set BlendShape value for an BlendShape with an invalid associated morph target name. BlendShape name: %s"), *StaticEnum<EFaceBlendShapePICO>()->GetValueAsString(BlendShape));
		return;
	}

	FName BlendShapeName = BlendShapeNameMapping[BlendShape];
	MorphTargetsManager.SetMeshMorphTargetValue(BlendShapeName, Value);
}

float UFaceTrackingComponentPICO::GetBlendShapeValue(EFaceBlendShapePICO BlendShape) const
{
	if (BlendShape >= EFaceBlendShapePICO::COUNT)
	{
		UE_LOG(PICOOpenXRMovement, Warning, TEXT("Cannot request BlendShape value using an invalid BlendShape index."));
		return 0.0f;
	}

	FName BlendShapeName = BlendShapeNameMapping[BlendShape];
	if (BlendShapeName == NAME_None)
	{
		UE_LOG(PICOOpenXRMovement, Warning, TEXT("Cannot request BlendShape value for an BlendShape with an invalid associated morph target name. BlendShape name: %s"), *StaticEnum<EFaceBlendShapePICO>()->GetValueAsString(BlendShape));
		return 0.0f;
	}

	return MorphTargetsManager.GetMeshMorphTargetValue(BlendShapeName);
}

void UFaceTrackingComponentPICO::ClearBlendShapeValues()
{
	MorphTargetsManager.EmptyMorphTargets();
}

bool UFaceTrackingComponentPICO::InitializeFaceTracking()
{
	FTTargetMeshComponent = FindComponentByNamePICO<USkinnedMeshComponent>(GetOwner(), FTTargetMeshComponentName);

	if (!IsValid(FTTargetMeshComponent))
	{
		UE_LOG(PICOOpenXRMovement, Warning, TEXT("Could not find skeletal mesh component with name: (%s). (%s:%s)"), *FTTargetMeshComponentName.ToString(), *GetOwner()->GetName(), *GetName());
		return false;
	}

	if (FTTargetMeshComponent && FTTargetMeshComponent->GetSkinnedAsset())
	{
		const TMap<FName, int32>& MorphTargetIndexMap = Cast<USkeletalMesh>(FTTargetMeshComponent->GetSkinnedAsset())->GetMorphTargetIndexMap();

		for (const auto& it : BlendShapeNameMapping)
		{
			ValidBlendShape[static_cast<int32>(it.Key)] = MorphTargetIndexMap.Contains(it.Value);
		}

		return true;
	}

	return false;
}

void FMorphTargetsManagerPICO::ResetMeshMorphTargetCurves(USkinnedMeshComponent* TargetMeshComponent)
{
	if (TargetMeshComponent)
	{
		TargetMeshComponent->ActiveMorphTargets.Reset();

		if (TargetMeshComponent->GetSkinnedAsset())
		{
			TargetMeshComponent->MorphTargetWeights.SetNum(TargetMeshComponent->GetSkinnedAsset()->GetMorphTargets().Num());
			if (TargetMeshComponent->MorphTargetWeights.Num() > 0)
			{
				FMemory::Memzero(TargetMeshComponent->MorphTargetWeights.GetData(), TargetMeshComponent->MorphTargetWeights.GetAllocatedSize());
			}
		}
		else
		{
			TargetMeshComponent->MorphTargetWeights.Reset();
		}
	}
}

void FMorphTargetsManagerPICO::UpdateMeshMorphTargets(USkinnedMeshComponent* TargetMeshComponent)
{
	if (TargetMeshComponent && TargetMeshComponent->GetSkinnedAsset())
	{
		if (MeshMorphTargetCurves.Num() > 0)
		{
			FAnimationRuntime::AppendActiveMorphTargets(Cast<USkeletalMesh>(TargetMeshComponent->GetSkinnedAsset()), MeshMorphTargetCurves, TargetMeshComponent->ActiveMorphTargets, TargetMeshComponent->MorphTargetWeights);
		}
	}
}

void FMorphTargetsManagerPICO::SetMeshMorphTargetValue(FName MorphTargetName, float Value)
{
	float* CurveValPtr = MeshMorphTargetCurves.Find(MorphTargetName);
	bool bShouldAddToList = FPlatformMath::Abs(Value) > ZERO_ANIMWEIGHT_THRESH;
	if (bShouldAddToList)
	{
		if (CurveValPtr)
		{
			*CurveValPtr = Value;
		}
		else
		{
			MeshMorphTargetCurves.Add(MorphTargetName, Value);
		}
	}
	else
	{
		MeshMorphTargetCurves.Remove(MorphTargetName);
	}
}

float FMorphTargetsManagerPICO::GetMeshMorphTargetValue(FName MorphTargetName) const
{
	const float* CurveValPtr = MeshMorphTargetCurves.Find(MorphTargetName);

	if (CurveValPtr)
	{
		return *CurveValPtr;
	}
	else
	{
		return 0.0f;
	}
}

void FMorphTargetsManagerPICO::EmptyMorphTargets()
{
	MeshMorphTargetCurves.Empty();
}
