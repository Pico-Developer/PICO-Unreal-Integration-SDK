// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "PICOSpatialMeshEntity.h"

/**
 * Static mesh entity proxy.
 * Represents a static mesh component in the spatial scene.
 * Handles static mesh data extraction and synchronization with the native spatial entity.
 */
class FStaticMeshEntityProxy : public FMeshEntityProxy
{
public:
	/**
	 * Constructor.
	 * @param InParentEntityProxy Parent entity proxy (can be null for root entities)
	 * @param SceneComponent The static mesh component this proxy represents
	 */
	FStaticMeshEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent);
	
	virtual ~FStaticMeshEntityProxy() {}

	/** Updates proxy data from the static mesh component */
	virtual void UpdateEntityProxy(USceneComponent* SceneComponent) override;
	
	/** Initializes the native spatial entity with static mesh data */
	virtual bool InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer) override;
	
	/** Updates the native entity with current static mesh proxy data */
	virtual void UpdateNativeEntity() override;

protected:
	/**
	 * Constructs static mesh-specific entity data.
	 * @param SceneComponent The scene component to extract static mesh data from
	 * @param bInitialize Whether to perform full initialization or partial update
	 */
	void ConstrucStaticMeshEntityData(USceneComponent* SceneComponent, bool bInitialize);

public:
	// Dynamic Data
	
	/** Path to the static mesh asset */
	FString StaticMesh;
	
	/** Whether the mesh asset has changed and needs updating */
	bool bMeshChanged;
};
