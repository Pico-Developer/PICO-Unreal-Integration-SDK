// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PICO_FaceTrackingComponent.h"
#include "Animation/AnimTypes.h"
#include "LiveLinkRetargetAsset.h"
#include "Containers/StaticArray.h"
#include "Misc/EngineVersionComparison.h"
#include "PICO_LiveLinkRetargetAsset.generated.h"

USTRUCT(BlueprintType)
struct PICOOPENXRMOVEMENT_API FPICOAnimCurveMapping
{
	GENERATED_BODY()

	FPICOAnimCurveMapping() {};

	FPICOAnimCurveMapping(const std::initializer_list<FName> CurveNamesList)
		: CurveNames(CurveNamesList)
	{
	}

	UPROPERTY(EditAnywhere, Category = "PICO|FaceTracking")
		TArray<FName> CurveNames;
};

UCLASS(Blueprintable, meta = (DisplayName = "LiveLink retarget face asset PICO"))
class PICOOPENXRMOVEMENT_API UFaceLiveLinkRetargetAssetPICO : public ULiveLinkRetargetAsset
{
	GENERATED_UCLASS_BODY()

	virtual void Initialize() override;
	virtual void BuildPoseAndCurveFromBaseData(float DeltaTime, const FLiveLinkBaseStaticData* InBaseStaticData, const FLiveLinkBaseFrameData* InBaseFrameData, FCompactPose& OutPose, FBlendedCurve& OutCurve) override;

	UPROPERTY(EditDefaultsOnly, Category = "PICO|FaceTracking")
		TMap<EFaceBlendShapePICO, FPICOAnimCurveMapping> CurveRemapping;

private:
	FGuid LastSkeletonGuid;

#if UE_VERSION_OLDER_THAN(5, 3, 0)
	TStaticArray<TArray<SmartName::UID_Type>, static_cast<uint8>(EPICOFaceBlendShape::COUNT)> RemappingForLastSkeleton;
#else
	TStaticArray<TArray<FName>, static_cast<uint8>(EFaceBlendShapePICO::COUNT)> RemappingForLastSkeleton;
#endif

	void OnSkeletonChanged(const USkeleton* Skeleton);
};
