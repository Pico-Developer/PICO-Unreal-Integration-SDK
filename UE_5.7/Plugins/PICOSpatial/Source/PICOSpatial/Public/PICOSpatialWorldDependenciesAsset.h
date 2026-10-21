#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PICOSpatialFunctionLibrary.h"
#include "PICOSpatialWorldDependenciesAsset.generated.h"

UCLASS(BlueprintType, meta = (DisplayName = "PICO Spatial World Dependencies"))
class PICOSPATIAL_API UPICOSpatialWorldDependenciesAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial|Dependencies")
	TMap<FName, FWorldDependenciesPICO> WorldDependencies;
};
