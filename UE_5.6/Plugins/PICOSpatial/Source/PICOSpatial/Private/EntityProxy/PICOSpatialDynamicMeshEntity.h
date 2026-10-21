// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "PICOSpatialMeshEntity.h"

/**
 * Dynamic mesh entity proxy.
 * Represents a dynamic mesh component in the spatial scene.
 * Handles runtime mesh modifications through UDynamicMesh and responds to mesh change events.
 */
class FDynamicMeshEntityProxy : public FMeshEntityProxy
{
public:
	/**
	 * Constructor.
	 * @param InParentEntityProxy Parent entity proxy (can be null for root entities)
	 * @param SceneComponent The dynamic mesh component this proxy represents
	 */
	FDynamicMeshEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent);
	
	virtual ~FDynamicMeshEntityProxy();

	/** Updates proxy data from the dynamic mesh component */
	virtual void UpdateEntityProxy(USceneComponent* SceneComponent) override;
	
	/** Initializes the native spatial entity with dynamic mesh data */
	virtual bool InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer) override;
	
	/** Updates the native entity with current dynamic mesh proxy data */
	virtual void UpdateNativeEntity() override;

	/** Rebuilds the entire dynamic mesh in the spatial entity */
	void RebuildDynamicMesh();

protected:
	/**
	 * Constructs dynamic mesh-specific entity data.
	 * @param SceneComponent The scene component to extract dynamic mesh data from
	 * @param bInitialize Whether to perform full initialization or partial update
	 */
	void ConstructDynamicMeshEntityData(USceneComponent* SceneComponent, bool bInitialize);

	/**
	 * Callback for when the dynamic mesh is modified.
	 * @param DynamicMesh The modified dynamic mesh
	 * @param ChangeInfo Information about what changed in the mesh
	 */
	void OnDynamicMeshModified(UDynamicMesh* DynamicMesh, FDynamicMeshChangeInfo ChangeInfo);

private:
	/** Pointer to the UDynamicMesh being tracked */
	UDynamicMesh* DynamicMesh;
	
	/** Whether the mesh needs to be rebuilt */
	bool bNeedRebuild;
	
	/** Delegate handle for mesh modification callback */
	FDelegateHandle DelegateHandle;
};
