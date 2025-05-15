// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#pragma once
#include "CoreMinimal.h"
#include "PXR_MRTypes.h"
#include "PXR_AnchorComponent.h"
#include "PXR_MRFunctionLibrary.generated.h"

UCLASS()
class PICOXRMR_API UPICOXRMRFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorEntityUuid(AActor* BoundActor, FPICOAnchorUUID& OutAnchorUUID);

	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorComponentFlags(AActor* BoundActor, TArray<TEnumAsByte<EPICOAnchorComponentTypeFlag::Type>>& OutAnchorComponentFlags);

	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorSceneLabel(AActor* BoundActor, EPICOAnchorSceneLabel& OutAnchorSceneLabel);

	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorPlaneBoundaryInfo(AActor* BoundActor, FPICOAnchorPlaneBoundaryInfo& OutAnchorPlaneBoundaryInfo);

	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorPlanePolygonInfo(AActor* BoundActor, FPICOAnchorPlanePolygonInfo& OutAnchorPlanePolygonInfo);

	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorVolumeInfo(AActor* BoundActor, FPICOAnchorVolumeInfo& OutAnchorVolumeInfo);

	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorPoseByComponent(UPICOAnchorComponent* BoundComponent, FTransform& OutTransform);

	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorPoseByActor(AActor* BoundActor, FTransform& OutTransform);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRMR", meta = (WorldContext = "WorldContext", UnsafeDuringActorConstruction = "true"))
	static AActor* PXR_SpawnActorFromLoadResult(UObject* WorldContext, const FAnchorLoadResult& LoadResult, UClass* ActorClass);

	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_IsAnchorValidForActor(AActor* BoundActor);
	
	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_IsAnchorValidForComponent(UActorComponent* BoundComponent);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (Anchor Handle)", CompactNodeTitle = "->", BlueprintAutocast), Category = "PXR|PXRMR")
	static FString PXR_FromAnchorToString(const FPICOAnchor& Anchor);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (Anchor UUID)", CompactNodeTitle = "->", BlueprintAutocast), Category = "PXR|PXRMR")
	static FString PXR_FromUUIDToString(const FPICOAnchorUUID& AnchorUUID);
};