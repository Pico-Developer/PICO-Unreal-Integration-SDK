// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "PICOSpatialMeshEntity.h"

/**
 * Skeletal mesh entity proxy.
 * Represents a skeletal mesh component in the spatial scene.
 * Handles skeletal mesh data and bone transformations for animated meshes.
 */
class FSkeletalMeshEntityProxy : public FMeshEntityProxy
{
public:
	/**
	 * Constructor.
	 * @param InParentEntityProxy Parent entity proxy (can be null for root entities)
	 * @param SceneComponent The skeletal mesh component this proxy represents
	 */
	FSkeletalMeshEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent);
	
	virtual ~FSkeletalMeshEntityProxy() {}

	/** Updates proxy data from the skeletal mesh component */
	virtual void UpdateEntityProxy(USceneComponent* SceneComponent) override;
	
	/** Initializes the native spatial entity with skeletal mesh data */
	virtual bool InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer) override;
	
	/** Updates the native entity with current skeletal mesh proxy data */
	virtual void UpdateNativeEntity() override;

protected:
	/**
	 * Constructs skeletal mesh-specific entity data.
	 * @param SceneComponent The scene component to extract skeletal mesh data from
	 * @param bInitialize Whether to perform full initialization or partial update
	 */
	void ConstructSkeletalMeshEntityData(USceneComponent* SceneComponent, bool bInitialize);

public:
	// Dynamic Data
	
	/** Path to the skeletal mesh asset */
	FString SkeletalMesh;
	
	/** Whether the skeletal mesh asset has changed and needs updating */
	bool bMeshChanged;
	
#if PLATFORM_ANDROID
	/** Bone transform bindings for skeletal animation */
	TArray<spatialpack::TransformBindValue> BoneTransforms;
#endif
};
