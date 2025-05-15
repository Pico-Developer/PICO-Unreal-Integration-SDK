// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#include "PXR_AnchorComponent.h"
#include "PXR_AnchorManager.h"

UPICOMREventComponent::UPICOMREventComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPICOMREventComponent::BeginPlay()
{
	Super::BeginPlay();

	PXR_LOGI(PxrMR, "UPICOMREventComponent::BeginPlay Bind OnSpatialTrackingStateUpdate Event");
	HandleOfSpatialTrackingStateUpdate = FPICOAnchorManager::GetInstance()->OnSpatialTrackingStateUpdate().AddUObject(this, &UPICOMREventComponent::OnSpatialTrackingStateUpdate);
}

void UPICOMREventComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPICOMREventComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	PXR_LOGI(PxrMR, "UPICOMREventComponent::EndPlay UnBind OnSpatialTrackingStateUpdate Event");
	FPICOAnchorManager::GetInstance()->OnSpatialTrackingStateUpdate().Remove(HandleOfSpatialTrackingStateUpdate);
}

UPICOAnchorComponent::UPICOAnchorComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AnchorHandle = 0;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UPICOAnchorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPICOAnchorComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (IsValid(GetOwner()) && IsValid(GetWorld()))
	{
		FPICOAnchorManager::GetInstance()->UpdateAnchor(this);
	}
}

void UPICOAnchorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (IsAnchorValid())
	{
		EPICOResult Result = EPICOResult::PXR_Error_ValidationFailure;
		FPICOAnchorManager::GetInstance()->DestroyAnchorEntity(GetOwner(), nullptr, Result);
	}
}