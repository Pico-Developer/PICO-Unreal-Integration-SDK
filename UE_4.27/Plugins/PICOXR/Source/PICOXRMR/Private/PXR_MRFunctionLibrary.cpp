// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#include "PXR_MRFunctionLibrary.h"
#include "PXR_AnchorManager.h"


bool UPICOXRMRFunctionLibrary::PXR_GetAnchorEntityUuid(AActor* BoundActor, FPICOAnchorUUID& OutAnchorUUID)
{
	return FPICOAnchorManager::GetInstance()->GetAnchorEntityUUID(BoundActor, OutAnchorUUID);
}

bool UPICOXRMRFunctionLibrary::PXR_GetAnchorComponentFlags(AActor* BoundActor, TArray<EPICOAnchorComponentTypeFlag>& OutAnchorComponentFlags)
{
	return FPICOAnchorManager::GetInstance()->GetAnchorComponentFlags(BoundActor, OutAnchorComponentFlags);
}

bool UPICOXRMRFunctionLibrary::PXR_GetAnchorSceneLabel(AActor* BoundActor, EPICOAnchorSceneLabel& OutAnchorSceneLabel)
{
	return FPICOAnchorManager::GetInstance()->GetAnchorSceneLabel(BoundActor, OutAnchorSceneLabel);
}

bool UPICOXRMRFunctionLibrary::PXR_GetAnchorPlaneBoundaryInfo(AActor* BoundActor, FPICOAnchorPlaneBoundaryInfo& OutAnchorPlaneBoundaryInfo)
{
	return FPICOAnchorManager::GetInstance()->GetAnchorPlaneBoundaryInfo(BoundActor, OutAnchorPlaneBoundaryInfo);
}

bool UPICOXRMRFunctionLibrary::PXR_GetAnchorPlanePolygonInfo(AActor* BoundActor, FPICOAnchorPlanePolygonInfo& OutAnchorPlanePolygonInfo)
{
	return FPICOAnchorManager::GetInstance()->GetAnchorPlanePolygonInfo(BoundActor, OutAnchorPlanePolygonInfo);
}

bool UPICOXRMRFunctionLibrary::PXR_GetAnchorVolumeInfo(AActor* BoundActor, FPICOAnchorVolumeInfo& OutAnchorVolumeInfo)
{
	return FPICOAnchorManager::GetInstance()->GetAnchorVolumeInfo(BoundActor, OutAnchorVolumeInfo);
}

bool UPICOXRMRFunctionLibrary::PXR_GetAnchorPoseByComponent(UPICOAnchorComponent* BoundComponent, FTransform& OutTransform)
{
	return FPICOAnchorManager::GetInstance()->GetAnchorPose(BoundComponent, OutTransform);
}

bool UPICOXRMRFunctionLibrary::PXR_GetAnchorPoseByActor(AActor* BoundActor, FTransform& OutTransform)
{
	if (!IsValid(BoundActor) || !IsValid(BoundActor->GetWorld()))
	{
		return false;
	}

	UPICOAnchorComponent* AnchorComponent = Cast<UPICOAnchorComponent>(BoundActor->GetComponentByClass(UPICOAnchorComponent::StaticClass()));
	return FPICOAnchorManager::GetInstance()->GetAnchorPose(AnchorComponent, OutTransform);
}

AActor* UPICOXRMRFunctionLibrary::PXR_SpawnActorFromLoadResult(UObject* WorldContext, const FAnchorLoadResult& LoadResult, UClass* ActorClass)
{
	PXR_LOGI(PxrMR, "UPICOXRMRFunctionLibrary::PXR_SpawnActorFromLoadResult Params: Handle[%llu], UUID[%s], Position[%d]", (uint64)LoadResult.AnchorHandle.GetValue(), *LoadResult.AnchorUUID.ToString(), (int32)LoadResult.PersistLocation);

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
	if (!IsValid(World))
	{
		PXR_LOGI(PxrMR, "UPICOXRMRFunctionLibrary::PXR_SpawnActorFromLoadResult is Invalid");
		return nullptr;
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.ObjectFlags |= RF_Transient;

	AActor* AnchorActor = World->SpawnActor(ActorClass, nullptr, nullptr, SpawnInfo);
	if (!IsValid(AnchorActor))
	{
		PXR_LOGI(PxrMR, "UPICOXRMRFunctionLibrary::PXR_SpawnActorFromLoadResult Spawn Actor Failed");
		return nullptr;
	}

	UPICOAnchorComponent* AnchorComponent = Cast<UPICOAnchorComponent>(AnchorActor->GetComponentByClass(UPICOAnchorComponent::StaticClass()));
	if (IsValid(AnchorComponent) && AnchorComponent->IsAnchorValid())
	{
		PXR_LOGI(PxrMR, "UPICOXRMRFunctionLibrary::PXR_SpawnActorFromLoadResult Anchor is Already Valid, Return Old Anchor Actor");
		return AnchorActor;
	}
	if (AnchorComponent == nullptr)
	{
		AnchorComponent = Cast<UPICOAnchorComponent>(AnchorActor->AddComponentByClass(UPICOAnchorComponent::StaticClass(), false, FTransform::Identity, false));
	}

	AnchorComponent->SetAnchorHandle(LoadResult.AnchorHandle);
	AnchorComponent->SetAnchorUUID(LoadResult.AnchorUUID);
	return AnchorActor;
}

bool UPICOXRMRFunctionLibrary::PXR_IsAnchorValidForActor(AActor* BoundActor)
{
	if (!IsValid(BoundActor))
	{
		return false;
	}

	UPICOAnchorComponent* AnchorComponent = Cast<UPICOAnchorComponent>(BoundActor->GetComponentByClass(UPICOAnchorComponent::StaticClass()));
	if (IsValid(AnchorComponent) && AnchorComponent->IsAnchorValid())
	{
		return true;
	}

	return false;
}

bool UPICOXRMRFunctionLibrary::PXR_IsAnchorValidForComponent(UActorComponent* BoundComponent)
{
	if (!IsValid(BoundComponent) || !BoundComponent->IsA(UPICOAnchorComponent::StaticClass()))
	{
		return false;
	}

	UPICOAnchorComponent* AnchorComponent = Cast<UPICOAnchorComponent>(BoundComponent);
	if (IsValid(AnchorComponent) && AnchorComponent->IsAnchorValid())
	{
		return true;
	}

	return false;
}

FString UPICOXRMRFunctionLibrary::PXR_FromAnchorToString(const FPICOAnchor& Anchor)
{
	return Anchor.ToString();
}

FString UPICOXRMRFunctionLibrary::PXR_FromUUIDToString(const FPICOAnchorUUID& AnchorUUID)
{
	return AnchorUUID.ToString();
}