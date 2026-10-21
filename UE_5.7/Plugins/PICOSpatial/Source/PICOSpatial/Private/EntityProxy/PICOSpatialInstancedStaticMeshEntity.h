// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "PICOSpatialStaticMeshEntity.h"

/**
 * Instanced static mesh entity proxy.
 * Represents an instanced static mesh component in the spatial scene.
 * Handles instance data extraction and synchronization for efficient rendering of multiple instances.
 */
class FInstancedStaticMeshEntityProxy : public FStaticMeshEntityProxy
{
public:
	/**
	 * Constructor.
	 * @param InParentEntityProxy Parent entity proxy (can be null for root entities)
	 * @param SceneComponent The instanced static mesh component this proxy represents
	 */
	FInstancedStaticMeshEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent);
	
	virtual ~FInstancedStaticMeshEntityProxy() {}

	/** Updates proxy data from the instanced static mesh component */
	virtual void UpdateEntityProxy(USceneComponent* SceneComponent) override;
	
	/** Initializes the native spatial entity with instance data */
	virtual bool InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer) override;
	
	/** Updates the native entity with current instance proxy data */
	virtual void UpdateNativeEntity() override;
	
protected:
	/**
	 * Constructs instanced static mesh-specific entity data.
	 * @param SceneComponent The scene component to extract instance data from
	 * @param bInitialize Whether to perform full initialization or partial update
	 */
	void ConstructInstancedStaticMeshEntityData(USceneComponent* SceneComponent, bool bInitialize);

	/** Whether render instances need updating */
	bool bRenderInstancesDirty;

public:
	/** Whether instances need complete reconstruction */
	bool bNeedReConstructInstances;

#if PLATFORM_ANDROID
	/** Native mesh instances resource for spatial rendering */
	spatialpack::MeshInstancesResource MeshInstancesResource;
#endif // PLATFORM_ANDROID
};
