// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialSkeletalMeshEntity.h"
#include "PICOSpatial.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/Package.h"

void FSkeletalMeshEntityProxy::ConstructSkeletalMeshEntityData(USceneComponent* SceneComponent, bool bInitialize)
{
	if (!bVisible)
	{
		return;
	}

	if (USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(SceneComponent))
	{
		if (bInitialize || SkeletalMeshComponent->VisibilityBasedAnimTickOption != EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones)
		{
			SkeletalMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
		}
		FString NewSkeletalMesh = "";

		if (USkeletalMesh* SKMesh = SkeletalMeshComponent->GetSkeletalMeshAsset())
		{
			NewSkeletalMesh = SKMesh->GetPackage()->GetPathName();
		}

		bMeshChanged = SkeletalMesh != NewSkeletalMesh;
		if (bMeshChanged)
		{
			SkeletalMesh = NewSkeletalMesh;
		}

#if PLATFORM_ANDROID
		int32 BoneCount = SkeletalMeshComponent->GetNumBones();
		if (BoneCount != BoneTransforms.Num())
		{
			BoneTransforms.SetNum(BoneCount);
		}
		for (int32 BoneIndex = 0; BoneIndex < BoneCount; ++BoneIndex)
		{
			const FTransform& BonePose = SkeletalMeshComponent->GetBoneTransform(SkeletalMeshComponent->GetBoneName(BoneIndex), ERelativeTransformSpace::RTS_ParentBoneSpace);
			BoneTransforms[BoneIndex].setTranslation(ToSpVector(BonePose.GetLocation(), 100.0f));
			BoneTransforms[BoneIndex].setRotation(ToSpQuat(BonePose.GetRotation()));
			BoneTransforms[BoneIndex].setScale(ToSpVector(BonePose.GetScale3D()));
		}
#endif
	}
}

FSkeletalMeshEntityProxy::FSkeletalMeshEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent)
	: FMeshEntityProxy(InParentEntityProxy, SceneComponent)
{
	ConstructSkeletalMeshEntityData(SceneComponent, true);
}

void FSkeletalMeshEntityProxy::UpdateEntityProxy(USceneComponent* SceneComponent)
{
	FMeshEntityProxy::UpdateEntityProxy(SceneComponent);
	ConstructSkeletalMeshEntityData(SceneComponent, false);
}

bool FSkeletalMeshEntityProxy::InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer)
{
	if (FMeshEntityProxy::InitializeNativeEntity(InCurrentScene, SpatialContainer))
	{
		if (NativeEntity.Get()->IsValid())
		{
			return true;
		}
	}
	return false;
}

void FSkeletalMeshEntityProxy::UpdateNativeEntity()
{
	FMeshEntityProxy::UpdateNativeEntity();
#if PLATFORM_ANDROID
	spatialpack::SceneEntity entity = NativeEntity.Get()->entity;
	if (!entity.isValid())
	{
		return;
	}
	if (bMeshChanged)
	{
		if (SkeletalMesh != "")
		{
			spatialpack::MeshResource* Mesh = GetCachedPICOSpatial().CacheMeshResource(SkeletalMesh);
			if (Mesh)
			{
				UE_LOG(LogPICOSpatial, Verbose, TEXT("Set SkeletalMeshComponent:%s, SkeletalMesh:%s, NativeMesh:%llu"), *Name, *SkeletalMesh, Mesh->getResourceId());
				entity.getOrAddModelComponent().setMesh(*Mesh);
			}
		}
		else
		{
			spatialpack::MeshResource Mesh;
			entity.getOrAddModelComponent().setMesh(Mesh);
		}
		bMeshChanged = false;
	}

	if (bVisible)
	{
		spatialpack::AnimationUtility::setJointTransforms(entity, BoneTransforms.GetData(), BoneTransforms.Num());
	}
#endif // PLATFORM_ANDROID
}
