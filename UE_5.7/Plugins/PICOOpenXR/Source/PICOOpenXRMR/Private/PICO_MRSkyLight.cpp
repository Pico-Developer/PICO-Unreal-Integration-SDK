// Fill out your copyright notice in the Description page of Project Settings.


#include "PICO_MRSkyLight.h"
#include "Engine/TextureCube.h"

AMRSkyLightPICO::AMRSkyLightPICO(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	
	GetLightComponent()->SourceType = SLS_SpecifiedCubemap;
	GetLightComponent()->Cubemap =LoadObject<UTextureCube>(nullptr, TEXT("/Engine/EngineResources/DefaultTextureCube.DefaultTextureCube"), nullptr, LOAD_None, nullptr);
	GetLightComponent()->bLowerHemisphereIsBlack = false;
}
