// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "PICOSpatialPrimitiveEntity.h"

/**
 * Material input parameter storage structure.
 * Stores scalar, color, and texture parameters for a material.
 */
struct FMaterialInputParameters
{
	/** Scalar parameter values keyed by parameter info */
	TMap<FMaterialParameterInfo, FMaterialParameterMetadata> ScalarParameters;
	
	/** Color parameter values keyed by parameter info */
	TMap<FMaterialParameterInfo, FMaterialParameterMetadata> ColorParameters;
	
	/** Texture parameter values keyed by parameter info */
	TMap<FMaterialParameterInfo, FMaterialParameterMetadata> TextureParameters;
};

/**
 * Mesh entity proxy.
 * Base class for all mesh-based entity proxies (static, skeletal, dynamic meshes).
 * Handles material management and parameter updates for mesh entities.
 */
class FMeshEntityProxy : public FPrimitiveEntityProxy
{
public:
	/**
	 * Constructor.
	 * @param InParentEntityProxy Parent entity proxy (can be null for root entities)
	 * @param SceneComponent The scene component this proxy represents
	 */
	FMeshEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent);
	
	virtual ~FMeshEntityProxy();

	/** Updates proxy data from the scene component */
	virtual void UpdateEntityProxy(USceneComponent* SceneComponent) override;
	
	/** Initializes the native spatial entity */
	virtual bool InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer) override;
	
	/** Updates the native entity with current proxy data */
	virtual void UpdateNativeEntity() override;

protected:
	/**
	 * Constructs mesh-specific entity data.
	 * @param SceneComponent The scene component to extract mesh data from
	 * @param bInitialize Whether to perform full initialization or partial update
	 */
	void ConstructMeshEntityData(USceneComponent* SceneComponent, bool bInitialize);

	/**
	 * Updates stored material parameters from a material interface.
	 * @param Material The material interface to extract parameters from
	 * @param UpdateStaticOnce Whether to update static parameters only once
	 */
	void UpdateMaterialStoredParameters(uint32 Index, class UMaterialInterface* Material, bool bIsStaticMaterialInstance, bool bIsDynamicMaterialInstance, bool UpdateStaticOnce = false);
	
	/**
	 * Updates native material parameters for a specific material slot.
	 * @param MaterialIndex The index of the material slot to update
	 */
	void UpdateNativeMaterialParameters(int MaterialIndex, spatialpack::SceneEntity& entity);
	
	/** Whether any material has changed and needs updating */
	bool bMaterialChanged;

private:
	/** Material paths for each material slot */
	TArray<FString> Materials;
	TArray<FString> DynamicMaterialInstances;
	static TMap<FString, TWeakObjectPtr<UMaterialInterface>> WeakDynamicMaterialInstances;
	TArray<bool> ShaderGraphFlags;
	/** Material parameters for each material slot */
	TArray<FMaterialInputParameters> MaterialParameters;
	void ProcessMaterialAtIndex(int32 Index, UMaterialInterface* SourceMaterial, bool bForceUpdate);
};
