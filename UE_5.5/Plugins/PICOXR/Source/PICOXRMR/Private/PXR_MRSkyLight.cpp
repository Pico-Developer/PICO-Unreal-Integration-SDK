// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_MRSkyLight.h"
#include "Engine/TextureCube.h"
#include "PXR_EventManager.h"
#include "PXR_HMDFunctionLibrary.h"


APICOMRSkyLight::APICOMRSkyLight(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	// Don't start ticking until we have a valid capture probe to update with
	PrimaryActorTick.bStartWithTickEnabled = false;
	// The environment probes don't update often so don't check very often
	PrimaryActorTick.TickInterval = 0.25f;

	GetLightComponent()->SourceType = SLS_SpecifiedCubemap;
	GetLightComponent()->Cubemap =LoadObject<UTextureCube>(nullptr, TEXT("/Engine/EngineResources/DefaultTextureCube.DefaultTextureCube"), nullptr, LOAD_None, nullptr);
	GetLightComponent()->bLowerHemisphereIsBlack = false;
}
