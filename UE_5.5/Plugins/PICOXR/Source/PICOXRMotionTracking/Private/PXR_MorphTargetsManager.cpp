// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_MorphTargetsManager.h"

#include "AnimationRuntime.h"
#include "Engine/SkinnedAsset.h"
#include "Engine/SkeletalMesh.h"

void FPXRMorphTargetsManager::ResetMeshMorphTargetCurves(USkinnedMeshComponent* TargetMeshComponent)
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

void FPXRMorphTargetsManager::UpdateMeshMorphTargets(USkinnedMeshComponent* TargetMeshComponent)
{
	if (TargetMeshComponent && TargetMeshComponent->GetSkinnedAsset())
	{
		if (MeshMorphTargetCurves.Num() > 0)
		{
			FAnimationRuntime::AppendActiveMorphTargets(Cast<USkeletalMesh>(TargetMeshComponent->GetSkinnedAsset()), MeshMorphTargetCurves, TargetMeshComponent->ActiveMorphTargets, TargetMeshComponent->MorphTargetWeights);
		}
	}
}

void FPXRMorphTargetsManager::SetMeshMorphTargetValue(FName MorphTargetName, float Value)
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

float FPXRMorphTargetsManager::GetMeshMorphTargetValue(FName MorphTargetName) const
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

void FPXRMorphTargetsManager::EmptyMorphTargets()
{
	MeshMorphTargetCurves.Empty();
}
