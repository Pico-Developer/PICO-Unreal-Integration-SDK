#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PICOSpatialAssetMappingsAsset.generated.h"

UCLASS(BlueprintType, meta = (DisplayName = "PICO Spatial Asset Mappings"))
class PICOSPATIAL_API UPICOSpatialAssetMappingsAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial|Mappings",
		meta = (ToolTip = "Key: UE asset package path (Package Path). Value: native bundle asset entry path."))
	TMap<FString, FString> MeshMappings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial|Mappings",
		meta = (ToolTip = "Key: UE asset package path (Package Path). Value: native bundle asset entry path."))
	TMap<FString, FString> MaterialMappings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial|Mappings",
		meta = (ToolTip = "Key: UE asset package path (Package Path). Value: native bundle asset entry path."))
	TMap<FString, FString> TextureMappings;
};

