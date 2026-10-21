// Fill out your copyright notice in the Description page of Project Settings.

#include "PICO_LiveLinkRetargetAsset.h"
#include "LiveLinkTypes.h"
#include "Algo/ForEach.h"
#include "Animation/AnimCurveTypes.h"
#include "BonePose.h"

UFaceLiveLinkRetargetAssetPICO::UFaceLiveLinkRetargetAssetPICO(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFaceLiveLinkRetargetAssetPICO::Initialize()
{
	LastSkeletonGuid.Invalidate();
#if UE_VERSION_OLDER_THAN(5, 3, 0)
	Algo::ForEach(RemappingForLastSkeleton, [](TArray<SmartName::UID_Type>& Arr) { Arr.Reset(); });
#else
	Algo::ForEach(RemappingForLastSkeleton, [](TArray<FName>& Arr) { Arr.Reset(); });
#endif
}

void UFaceLiveLinkRetargetAssetPICO::BuildPoseAndCurveFromBaseData(float DeltaTime, const FLiveLinkBaseStaticData* InBaseStaticData, const FLiveLinkBaseFrameData* InBaseFrameData, FCompactPose& OutPose, FBlendedCurve& OutCurve)
{
	check(InBaseFrameData);
	if (static_cast<int32>(EFaceBlendShapePICO::COUNT) != InBaseFrameData->PropertyValues.Num())
	{
		UE_LOG(PICOOpenXRMovement, Error, TEXT("Received wrong data of live link frame. This retargeting asset must be used with Meta MovementSDK Live Link source and Face subject. (received %d face expressions, expected %d)"), InBaseFrameData->PropertyValues.Num(), static_cast<int32>(EFaceBlendShapePICO::COUNT));
		return;
	}
	const USkeleton* Skeleton = OutPose.GetBoneContainer().GetSkeletonAsset();
	if (!IsValid(Skeleton))
	{
		UE_LOG(PICOOpenXRMovement, Error, TEXT("No skeleton asset for this retargeting."));
		return;
	}
	if (LastSkeletonGuid != Skeleton->GetGuid())
	{
		OnSkeletonChanged(Skeleton);
	}

	for (uint8 ExpressionId = 0; ExpressionId < static_cast<uint8>(EFaceBlendShapePICO::COUNT); ++ExpressionId)
	{
#if UE_VERSION_OLDER_THAN(5, 3, 0)
		for (const SmartName::UID_Type UID : RemappingForLastSkeleton[ExpressionId])
		{
			OutCurve.Set(UID, InBaseFrameData->PropertyValues[ExpressionId]);
		}
#else
		for (const FName Name : RemappingForLastSkeleton[ExpressionId])
		{
			OutCurve.Set(Name, InBaseFrameData->PropertyValues[ExpressionId]);
		}
#endif
	}
}

void UFaceLiveLinkRetargetAssetPICO::OnSkeletonChanged(const USkeleton* Skeleton)
{
	Initialize();

	for (const auto& [ExpressionId, CurveMapping] : CurveRemapping)
	{
		for (const auto& CurveName : CurveMapping.CurveNames)
		{
#if UE_VERSION_OLDER_THAN(5, 3, 0)
			if (const SmartName::UID_Type UID = Skeleton->GetUIDByName(USkeleton::AnimCurveMappingName, CurveName); UID != SmartName::MaxUID)
			{
				RemappingForLastSkeleton[static_cast<uint8>(ExpressionId)].Emplace(UID);
			}
#else
			RemappingForLastSkeleton[static_cast<uint8>(ExpressionId)].Emplace(CurveName);
#endif
		}
	}

	LastSkeletonGuid = Skeleton->GetGuid();
}
