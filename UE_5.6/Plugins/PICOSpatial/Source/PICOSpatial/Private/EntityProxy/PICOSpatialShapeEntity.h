// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "PICOSpatialPrimitiveEntity.h"

/**
 * Shape entity proxy.
 * Represents basic geometric shape components in the spatial scene.
 * Handles primitive shapes like boxes, spheres, capsules, etc.
 */
class FShapeEntityProxy : public FPrimitiveEntityProxy
{
public:
	/**
	 * Constructor.
	 * @param InParentEntityProxy Parent entity proxy (can be null for root entities)
	 * @param SceneComponent The shape component this proxy represents
	 */
	FShapeEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent);
	
	virtual ~FShapeEntityProxy() {}

	/** Updates proxy data from the shape component */
	virtual void UpdateEntityProxy(USceneComponent* SceneComponent) override;
	
	/** Initializes the native spatial entity with shape data */
	virtual bool InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer) override;
	
	/** Updates the native entity with current shape proxy data */
	virtual void UpdateNativeEntity() override;

protected:
	/**
	 * Constructs shape-specific entity data.
	 * @param SceneComponent The scene component to extract shape data from
	 * @param bInitialize Whether to perform full initialization or partial update
	 */
	void ConstructShapeEntityData(USceneComponent* SceneComponent, bool bInitialize);

	// Static Data

	// Dynamic Data

	// Spatial Resource
};
