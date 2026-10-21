// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "PICOSpatialSceneEntity.h"

/**
 * Directional light entity proxy.
 * Represents a directional light component in the spatial scene.
 * Handles directional lighting properties like intensity, color, and shadow casting.
 */
class FDirectionalLightEntityProxy : public FSceneEntityProxy
{
public:
	/**
	 * Constructor.
	 * @param ParentEntity Parent entity proxy (can be null for root entities)
	 * @param SceneComponent The directional light component this proxy represents
	 */
	FDirectionalLightEntityProxy(FSceneEntityProxy* ParentEntity, USceneComponent* SceneComponent);
	
	virtual ~FDirectionalLightEntityProxy() {}

	/** Updates proxy data from the directional light component */
	virtual void UpdateEntityProxy(USceneComponent* SceneComponent) override;
	
	/** Initializes the native spatial entity with light data */
	virtual bool InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer) override;
	
	/** Updates the native entity with current light proxy data */
	virtual void UpdateNativeEntity() override;

protected:
	/**
	 * Constructs directional light-specific entity data.
	 * @param SceneComponent The scene component to extract light data from
	 * @param bInitialize Whether to perform full initialization or partial update
	 */
	void ConstructDirectionalLightEntityData(USceneComponent* SceneComponent, bool bInitialize);

public:
	// Static Data
	
	/** Whether the light casts shadows */
	bool bCastShadow;
	
	/** Light intensity in lux */
	float Lux;
	
	/** Light color */
	spatialpack::Color4f LightColor;

	// Dynamic Data

	// Spatial Resource
	
#if PLATFORM_ANDROID
	/** Offset entity for directional light positioning */
	spatialpack::SceneEntity DirectionalLightOffsetEntity;
#endif
};
