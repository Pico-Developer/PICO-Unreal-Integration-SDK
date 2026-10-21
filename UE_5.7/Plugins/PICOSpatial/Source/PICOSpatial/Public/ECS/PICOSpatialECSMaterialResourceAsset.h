#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#if PLATFORM_ANDROID
#pragma push_macro("INVALID_OBJECT")
#pragma push_macro("IN")
#pragma push_macro("OUT")
#undef INVALID_OBJECT
#undef IN
#undef OUT
#include "SpatialPack.h"
#pragma pop_macro("OUT")
#pragma pop_macro("IN")
#pragma pop_macro("INVALID_OBJECT")
#endif

#include "PICOSpatialECSMaterialResourceAsset.generated.h"

class UMaterialInterface;

UENUM(BlueprintType)
enum class EPICOSpatialECSMaterialSourceType : uint8
{
	FromUEMaterial UMETA(DisplayName = "From UE Material"),
	LoadFromBundle UMETA(DisplayName = "Load From Bundle"),
};

UCLASS(BlueprintType, meta = (DisplayName = "PICO Spatial ECS Material Resource Asset"))
class PICOSPATIAL_API UPICOSpatialECSMaterialResourceAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial|ECS|Material")
	EPICOSpatialECSMaterialSourceType SourceType = EPICOSpatialECSMaterialSourceType::FromUEMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial|ECS|Material")
	TObjectPtr<UMaterialInterface> Material = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial|ECS|Material")
        FString BundleEntryPath;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Material")
	bool ResolveNativeMaterial(bool bForceRecreate = false);

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Material")
	bool isNativeMaterialValid() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Material")
	int64 getNativeMaterialId() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Material")
	FString getNativeMaterialName() const;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Material")
        bool useUEMaterial(UMaterialInterface* InMaterial);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Material")
        bool loadFromBundle(const FString& InBundleEntryPath);

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Material")
	UMaterialInterface* getUEMaterial() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Material")
	FString getUEMaterialAssetPath() const;

        UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Material")
        FString getBundleEntryPath() const;

#if PLATFORM_ANDROID
	const spatialpack::Material& GetOrCreateNativeMaterial(bool bForceRecreate = false);
#endif

private:
#if PLATFORM_ANDROID
	bool BuildNativeMaterial();
	mutable spatialpack::Material NativeMaterial;
	mutable bool bHasNativeMaterial = false;
#endif
};
