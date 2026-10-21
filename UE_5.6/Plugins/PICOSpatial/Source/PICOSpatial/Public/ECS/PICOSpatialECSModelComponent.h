#pragma once

#include "CoreMinimal.h"
#include "ECS/PICOSpatialECSComponent.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EntityProxy/PICOSpatialMeshEntity.h"
#include "PICOSpatialECSModelComponent.generated.h"

class UMaterialInterface;
class UPICOSpatialECSMeshResourceAsset;
class UPICOSpatialECSMaterialResourceAsset;

UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class PICOSPATIAL_API UPICOSpatialECSModelComponent : public UPICOSpatialECSComponent
{
	GENERATED_BODY()

public:
	virtual void BeginDestroy() override;

	virtual EPICOSpatialECSComponentType GetComponentType() const override
	{
		return EPICOSpatialECSComponentType::Model;
	}

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Model Component")
	void setMeshResource(UPICOSpatialECSMeshResourceAsset* InMeshResource);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Model Component")
        void setMaterialResource(UPICOSpatialECSMaterialResourceAsset* InMaterial, int32 MaterialIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Model Component")
	void clearMaterials();

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Model Component")
	int32 getMaterialCount() const;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Model Component")
	void setAutoInstancingEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Model Component")
	bool getAutoInstancingEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Model Component")
	void setVisible(bool bInVisible);

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Model Component")
	bool isVisible() const;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Model Component")
	void setVisualBoundsMargin(FVector InBoundsMargin);

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Model Component")
	FVector getVisualBoundsMargin() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Model Component")
	UPICOSpatialECSMeshResourceAsset* getMeshResource() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Model Component")
        UPICOSpatialECSMaterialResourceAsset* getMaterialResource(int32 MaterialIndex = 0) const;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Model Component")
	void UpdateMaterialParameters();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Model Component")
	void UpdateMaterialParameter(int32 MaterialIndex);

#if PLATFORM_ANDROID
protected:
	virtual void OnInitializeNativeComponent(spatialpack::SceneEntity& InNativeEntity) override;
	virtual void ApplyAllPropertiesToNative() override;

private:
	void UpdateMaterialStoredParameters(uint32 Index, UMaterialInterface* Material, bool bIsStaticMaterialInstance, bool bIsDynamicMaterialInstance, bool UpdateStaticOnce);
	void UpdateNativeMaterialParameters(int MaterialIndex, spatialpack::SceneEntity& entity);
	void ProcessMaterialAtIndex(int32 Index, UMaterialInterface* SourceMaterial, bool bForceUpdate);

	TArray<FString> DynamicMaterialInstances;
	TArray<bool> ShaderGraphFlags;
	TArray<FMaterialInputParameters> MaterialParameters;
	TArray<FString> MaterialPaths;

	static TMap<FString, TWeakObjectPtr<UMaterialInterface>> WeakDynamicMaterialInstances;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Component")
	TObjectPtr<UPICOSpatialECSMeshResourceAsset> MeshResource = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Component")
        TArray<TObjectPtr<UPICOSpatialECSMaterialResourceAsset>> Materials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Component")
	bool bAutoInstancingEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Component")
	bool bVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Component")
	FVector VisualBoundsMargin = FVector::ZeroVector;
};
