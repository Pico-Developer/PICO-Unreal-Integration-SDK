// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARSkyLight.h"
#include "PICO_MRSkyLight.generated.h"

/**
 * 
 */
UCLASS(BlueprintType,DisplayName="Light Estimation SkyLight PICO")
class AMRSkyLightPICO : public ASkyLight
{
	GENERATED_BODY()
	AMRSkyLightPICO(const FObjectInitializer& ObjectInitializer);
};
