// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_AnchorComponent.h"
#include "PICO_SpatialAnchor.h"

UAnchorComponentPICO::UAnchorComponentPICO(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AnchorHandle = 0;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UAnchorComponentPICO::BeginPlay()
{
	Super::BeginPlay();
}

void UAnchorComponentPICO::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (IsAnchorValid())
	{

		FSpatialAnchorExtensionPICO::GetInstance()->UpdateAnchor(this);
	}
}

void UAnchorComponentPICO::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (IsAnchorValid())
	{
		EResultPICO OutResult=EResultPICO::XR_Error_Unknown_PICO;
		FSpatialAnchorExtensionPICO::GetInstance()->DestroyAnchorByHandle(AnchorHandle,OutResult);
		ResetAnchorHandle();
	}
}

void UAnchorComponentPICO::ResetAnchorHandle()
{
	AnchorHandle.Reset();
}
