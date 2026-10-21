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

#include "PICOSpatialECSMeshResourceAsset.generated.h"

class UStaticMesh;

UENUM(BlueprintType)
enum class EPICOSpatialECSMeshSourceType : uint8
{
	FromUEMesh UMETA(DisplayName = "From UE Mesh"),
	LoadFromBundle UMETA(DisplayName = "Load From Bundle"),
};

UCLASS(BlueprintType, meta = (DisplayName = "PICO Spatial ECS Mesh Resource Asset"))
class PICOSPATIAL_API UPICOSpatialECSMeshResourceAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial|ECS|MeshResource")
        EPICOSpatialECSMeshSourceType SourceType = EPICOSpatialECSMeshSourceType::FromUEMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial|ECS|MeshResource")
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Spatial|ECS|MeshResource")
        FString BundleEntryPath;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|MeshResource")
	bool ResolveNativeMeshResource(bool bForceRecreate = false);

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|MeshResource")
	bool isNativeMeshResourceValid() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|MeshResource")
	int64 getNativeMeshResourceId() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|MeshResource")
	FString getNativeMeshResourceName() const;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|MeshResource")
	bool getNativeMeshBounds(FVector& OutCenterCm, FVector& OutHalfExtentCm);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|MeshResource")
        bool useUEMesh(UStaticMesh* InMesh);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|MeshResource")
        bool loadFromBundle(const FString& InBundleEntryPath);

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|MeshResource")
	UStaticMesh* getMesh() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|MeshResource")
	FString getMeshAssetPath() const;

        UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|MeshResource")
        FString getBundleEntryPath() const;

#if PLATFORM_ANDROID
	const spatialpack::MeshResource& GetOrCreateNativeMeshResource(bool bForceRecreate = false);
#endif

private:
#if PLATFORM_ANDROID
	bool BuildNativeMeshResource();
	mutable spatialpack::MeshResource NativeMeshResource;
	mutable bool bHasNativeMeshResource = false;
#endif
};
